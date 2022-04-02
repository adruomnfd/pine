#include <impl/integrator/sppm.h>
#include <core/lowdiscrepancy.h>
#include <core/scene.h>
#include <util/distribution.h>
#include <util/parameters.h>

namespace pine {

SPPMIntegrator::SPPMIntegrator(const Parameters& params, Scene* scene)
    : RayIntegrator(params, scene) {
    initialSearchRadius = params.GetFloat("initialSearchRadius", 0.1f);
    nIterations = params.GetInt("nIterations", samplesPerPixel);
    photonsPerIteration = params.GetInt("photonsPerIteration", filmSize.x * filmSize.y);
    finalGatheringDepth = params.GetInt("finalGatheringDepth", 0);
    writeFrequency = params.GetInt("writeFrequency", -1);
    if (writeFrequency <= 0)
        writeFrequency = std::numeric_limits<int>::max();
}

struct SPPMPixel {
    struct VisiblePoint {
        vec3 p;
        vec3 wo;
        const Material* material = nullptr;
        Spectrum beta;
    } vp;

    Spectrum Ld;
    float radius = 0;
    AtomicFloat phi[Spectrum::nSamples];
    std::atomic<int> M{0};
    float N = 0;
    Spectrum tau;
};

struct SPPMPixelListNode {
    SPPMPixel* pixel = nullptr;
    SPPMPixelListNode* next = nullptr;
};

static bool ToGrid(vec3 p, AABB bounds, const int gridRes[3], vec3i& pi) {
    bool inBounds = true;
    vec3 pg = bounds.Offset(p);

    for (int i = 0; i < 3; i++) {
        pi[i] = int(gridRes[i] * pg[i]);
        inBounds &= pi[i] >= 0 && pi[i] < gridRes[i];
        pi[i] = Clamp(pi[i], 0, gridRes[i] - 1);
    }

    return inBounds;
}

static inline unsigned int HashVec3i(vec3i p, int hashSize) {
    return (unsigned int)((p.x * 73856093) ^ (p.y * 19349663) ^ (p.z * 83492791)) % hashSize;
}

void SPPMIntegrator::Render() {
    if (scene->lights.size() == 0)
        LOG_FATAL("[SPPMIntegrator][Render]No light in the scene");
    int nPixels = Area(filmSize);
    std::unique_ptr<SPPMPixel[]> pixels(new SPPMPixel[nPixels]);
    for (int i = 0; i < nPixels; i++)
        pixels[i].radius = initialSearchRadius;

    ProgressReporter pr("Rendering", "SPPMIterations", "Photons", nIterations, photonsPerIteration);

    int hashSize = nPixels;
    std::vector<std::vector<SPPMPixelListNode>> grids = {std::vector<SPPMPixelListNode>(hashSize)};

    for (int iter = 0; iter < nIterations; iter++) {
        ScopedPR(pr, iter, iter == nIterations - 1);

        {
            Profiler _("Accumulating Visible Points");
            ParallelFor(filmSize, [&](vec2i p) {
                auto& sampler = samplers[threadIdx];
                sampler.StartPixel(p, iter);

                SPPMPixel& pixel = pixels[p.x + filmSize.x * p.y];

                vec2 pFilm = (p + sampler.Get2D()) / filmSize;
                Ray ray = scene->camera.GenRay(pFilm, sampler.Get2D());

                Spectrum beta(1.0f);
                float bsdfPdf = 0.0f;
                int diffuseBounces = 0;

                for (int depth = 0; depth < maxDepth; depth++) {
                    Interaction it;
                    if (!Intersect(ray, it)) {
                        if (scene->envLight)
                            pixel.Ld += scene->envLight->Color(ray.d);
                        break;
                    }

                    if (!it.material) {
                        ray = it.SpawnRay(ray.d);
                        --depth;
                        continue;
                    }

                    it.n = it.material->BumpNormal(MaterialEvalCtx(it, -ray.d));

                    MaterialEvalCtx mc(it, -ray.d);
                    if (it.material->Is<EmissiveMaterial>()) {
                        Spectrum le = it.material->Le(mc);
                        if (depth == 0) {
                            pixel.Ld += beta * le;
                        } else {
                            float lightPdf = it.shape->Pdf(ray, it);
                            pixel.Ld += beta * le * PowerHeuristic(1, bsdfPdf, 1, lightPdf);
                        }
                        break;
                    }

                    if (depth == maxDepth - 1)
                        break;

                    pixel.Ld += beta * EstimateDirect(ray, it, sampler);

                    mc.u1 = sampler.Get1D();
                    mc.u2 = sampler.Get2D();
                    if (auto bs = it.material->Sample(mc)) {
                        if (!bs->isSpecular) {
                            ++diffuseBounces;
                            pixel.vp = {it.p, -ray.d, it.material, beta};
                        }
                        if (diffuseBounces > finalGatheringDepth || depth + 1 == maxDepth)
                            break;

                        beta *= AbsDot(bs->wo, it.n) * bs->f / bs->pdf;
                        bsdfPdf = bs->pdf;
                        ray = it.SpawnRay(bs->wo);
                    } else {
                        break;
                    }
                }
            });
        }

        std::vector<std::atomic<SPPMPixelListNode*>> grid(hashSize);
        AABB gridBounds;
        int gridRes[3];
        {
            Profiler _("Visible Point Grid Construction");
            float maxRadius = 0.0f;
            for (int i = 0; i < nPixels; i++) {
                const SPPMPixel& pixel = pixels[i];
                if (pixel.vp.beta.IsBlack())
                    continue;
                AABB vpBound =
                    AABB(pixel.vp.p - vec3(pixel.radius), pixel.vp.p + vec3(pixel.radius));
                gridBounds = Union(gridBounds, vpBound);
                maxRadius = std::max(maxRadius, pixel.radius);
            }

            vec3 diag = gridBounds.Diagonal();
            float maxDiag = max(diag.x, diag.y, diag.z);
            int baseGridRes = maxDiag / maxRadius;
            for (int i = 0; i < 3; i++)
                gridRes[i] = std::max((int)(baseGridRes * diag[i] / maxDiag), 1);
        }

        {
            Profiler _("Add Visible Point To SPPM Grid");
            uint64_t chunkIndex = 0, nodeIndex = 0;
            for (int i = 0; i < nPixels; i++) {
                SPPMPixel& pixel = pixels[i];
                if (!pixel.vp.beta.IsBlack()) {
                    float radius = pixel.radius;
                    vec3i pmin, pmax;
                    ToGrid(pixel.vp.p - vec3(radius), gridBounds, gridRes, pmin);
                    ToGrid(pixel.vp.p + vec3(radius), gridBounds, gridRes, pmax);
                    for (int z = pmin.z; z <= pmax.z; z++)
                        for (int y = pmin.y; y <= pmax.y; y++)
                            for (int x = pmin.x; x <= pmax.x; x++) {
                                unsigned int h = HashVec3i({x, y, z}, hashSize);
                                if (nodeIndex >= grids[chunkIndex].size()) {
                                    ++chunkIndex;
                                    nodeIndex = 0;
                                    if (chunkIndex >= grids.size())
                                        grids.push_back(std::vector<SPPMPixelListNode>(hashSize));
                                }
                                SPPMPixelListNode* node = &grids[chunkIndex][nodeIndex++];
                                node->pixel = &pixel;
                                node->next = grid[h];
                                grid[h] = node;
                            }
                }
            }
        }

        {
            Profiler _("Trace/Accumulate Photons");
            ParallelFor(photonsPerIteration, [&](int photonIndex) {
                uint64_t haltonIndex = (uint64_t)iter * photonsPerIteration + photonIndex;
                int haltonDim = 0;
                auto [light, lightPdf] =
                    lightSampler.SampleLight(RadicalInverse(haltonDim++, haltonIndex));
                vec2 up, ud;
                up[0] = RadicalInverse(haltonDim++, haltonIndex);
                up[1] = RadicalInverse(haltonDim++, haltonIndex);
                ud[0] = RadicalInverse(haltonDim++, haltonIndex);
                ud[1] = RadicalInverse(haltonDim++, haltonIndex);
                auto les = light->SampleLe(up, ud);
                Ray photonRay = les.ray;
                Spectrum beta = les.Le / (lightPdf * les.pdf);
                if (beta.IsBlack())
                    return;

                for (int depth = 0; depth < maxDepth; depth++) {
                    Interaction it;
                    if (!Intersect(photonRay, it))
                        break;

                    if (!it.material) {
                        --depth;
                        photonRay = it.SpawnRay(photonRay.d);
                        continue;
                    }

                    it.n = it.material->BumpNormal(MaterialEvalCtx(it, -photonRay.d));

                    if (depth > 0) {
                        vec3i photonGridIndex;
                        if (ToGrid(it.p, gridBounds, gridRes, photonGridIndex)) {
                            int h = HashVec3i(photonGridIndex, hashSize);
                            for (SPPMPixelListNode* node = grid[h]; node != nullptr;
                                 node = node->next) {
                                SPPMPixel& pixel = *node->pixel;
                                float radius = pixel.radius;
                                if (DistanceSquared(pixel.vp.p, it.p) > radius * radius)
                                    continue;
                                vec3 wi = -photonRay.d;
                                MaterialEvalCtx mc(it, pixel.vp.wo, wi);
                                Spectrum phi = beta * pixel.vp.material->F(mc);
                                for (int i = 0; i < Spectrum::nSamples; i++)
                                    pixel.phi[i].Add(phi[i]);
                                ++pixel.M;
                            }
                        }
                    }

                    vec3 wi = -photonRay.d;
                    vec2 ubsdf2;
                    float ubsdf1;
                    ubsdf2[0] = RadicalInverse(haltonDim++, haltonIndex);
                    ubsdf2[1] = RadicalInverse(haltonDim++, haltonIndex);
                    ubsdf1 = RadicalInverse(haltonDim++, haltonIndex);
                    MaterialEvalCtx mc(it, wi, vec3(0.0f), ubsdf2, ubsdf1);
                    auto bs = it.material->Sample(mc);
                    if (!bs)
                        break;
                    Spectrum bnew = beta * bs->f * AbsDot(bs->wo, it.n) / bs->pdf;

                    float q = std::max(1.0f - bnew.y() / beta.y(), 0.0f);
                    if (RadicalInverse(haltonDim++, haltonIndex) < q)
                        break;
                    beta = bnew / (1.0f - q);

                    photonRay = it.SpawnRay(bs->wo);
                }
            });
        }

        {
            Profiler _("Update Pixel Values From Photons");
            for (int i = 0; i < nPixels; i++) {
                SPPMPixel& p = pixels[i];
                if (p.M > 0.0f) {
                    float gamma = 1.5f / 3.0f;
                    float Nnew = p.N + gamma * p.M;
                    float Rnew = p.radius * std::sqrt(Nnew / (p.N + p.M));
                    Spectrum phi;
                    for (int j = 0; j < Spectrum::nSamples; j++)
                        phi[j] = p.phi[j];
                    p.tau = (p.tau + p.vp.beta * phi) * Sqr(Rnew) / Sqr(p.radius);
                    p.N = Nnew;
                    p.radius = Rnew;
                    p.M = 0.0f;
                    for (int j = 0; j < Spectrum::nSamples; j++)
                        p.phi[j] = 0.0f;
                }
                p.vp.beta = 0.0f;
                p.vp.material = nullptr;
            }
        }

        // Write Image
        if (iter + 1 == nIterations || ((iter + 1) % writeFrequency) == 0) {
            film->Clear();
            uint64_t Np = (uint64_t)(iter + 1) * (uint64_t)photonsPerIteration;
            for (int i = 0; i < nPixels; i++) {
                const SPPMPixel& pixel = pixels[i];
                Spectrum L = pixel.Ld / (iter + 1);
                L += pixel.tau / (Np * Pi * Sqr(pixel.radius));
                vec3 rgb = L.ToRGB();
                film->GetPixel({i % filmSize.x, i / filmSize.x}).rgb[0].Add(rgb[0]);
                film->GetPixel({i % filmSize.x, i / filmSize.x}).rgb[1].Add(rgb[1]);
                film->GetPixel({i % filmSize.x, i / filmSize.x}).rgb[2].Add(rgb[2]);
                film->GetPixel({i % filmSize.x, i / filmSize.x}).weight = 1.0f;
            }
            film->Finalize();
        }

    }  // nIteration
}

}  // namespace pine