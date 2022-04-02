#include <core/integrator.h>
#include <core/sampling.h>
#include <core/scene.h>
#include <core/color.h>
#include <util/parallel.h>
#include <util/profiler.h>
#include <util/fileio.h>
#include <impl/integrator/ao.h>
#include <impl/integrator/viz.h>
#include <impl/integrator/mlt.h>
#include <impl/integrator/path.h>
#include <impl/integrator/sppm.h>
#include <impl/integrator/bdpt.h>
#include <impl/integrator/lightpath.h>
#include <impl/integrator/randomwalk.h>

namespace pine {

std::shared_ptr<Integrator> Integrator::Create(const Parameters& params, Scene* scene) {
    std::string type = params.GetString("type");
    SWITCH(type) {
        CASE("AO") return std::make_shared<AOIntegrator>(params, scene);
        CASE("Viz") return std::make_shared<VizIntegrator>(params, scene);
        CASE("Mlt") return std::make_shared<MltIntegrator>(params, scene);
        CASE("Path") return std::make_shared<PathIntegrator>(params, scene);
        CASE("Sppm") return std::make_shared<SPPMIntegrator>(params, scene);
        CASE("Bdpt") return std::make_shared<BDPTIntegrator>(params, scene);
        CASE("LightPath") return std::make_shared<LightPathIntegrator>(params, scene);
        CASE("RandomWalk") return std::make_shared<RandomWalkIntegrator>(params, scene);
        DEFAULT {
            LOG_WARNING("[Integrator][Create]Unknown type \"&\"", type);
            return std::make_shared<PathIntegrator>(params, scene);
        }
    }
}
Integrator::Integrator(const Parameters& params, Scene* scene) : scene(scene) {
    film = &scene->camera.GetFilm();
    filmSize = scene->camera.GetFilm().Size();

    lightSampler = LightSampler::Create(params["lightSampler"], scene->lights);

    Parameters samplerParams = params["sampler"];
    samplerParams.Set("filmSize", filmSize);
    samplers = {Sampler::Create(samplerParams)};
    for (int i = 0; i < NumThreads() - 1; i++)
        samplers.push_back(samplers[0].Clone());
    samplesPerPixel = samplers[0].SamplesPerPixel();
}

RayIntegrator::RayIntegrator(const Parameters& params, Scene* scene)
    : Integrator(params, scene), accel(Accel::Create(params["accel"])) {
    accel->Initialize(scene);
    maxDepth = params.GetInt("maxDepth", 4);
}
bool RayIntegrator::Hit(Ray ray) const {
    SampledProfiler _(ProfilePhase::IntersectShadow);

    return accel->Hit(ray);
}
bool RayIntegrator::Intersect(Ray& ray, Interaction& it) const {
    SampledProfiler _(ProfilePhase::IntersectClosest);

    it.wi = -ray.d;
    return accel->Intersect(ray, it);
}
Spectrum RayIntegrator::IntersectTr(Ray ray, Sampler& sampler) const {
    SampledProfiler _(ProfilePhase::IntersectTr);

    vec3 p2 = ray(ray.tmax);
    Interaction it;
    Spectrum tr = Spectrum(1.0f);

    while (true) {
        bool hitSurface = Intersect(ray, it);
        if (ray.medium)
            tr *= ray.medium->Tr(ray, sampler);
        if (hitSurface && it.material)
            return Spectrum(0.0f);
        if (!hitSurface)
            break;
        ray = it.SpawnRayTo(p2);
    }

    return tr;
}
Spectrum RayIntegrator::EstimateDirect(Ray ray, Interaction it, Sampler& sampler) const {
    SampledProfiler _(ProfilePhase::EstimateDirect);

    auto [light, lightPdf] = lightSampler.SampleLight(it.p, it.n, sampler.Get1D());
    if (!light)
        return Spectrum(0.0f);
    LightSample ls = light->Sample(it.p, sampler.Get2D());
    ls.pdf *= lightPdf;

    Spectrum tr = IntersectTr(it.SpawnRay(ls.wo, ls.distance), sampler);
    if (tr.IsBlack())
        return Spectrum(0.0f);

    Spectrum f;
    float scatteringPdf = 0.0f;
    if (it.IsSurfaceInteraction()) {
        MaterialEvalCtx mc(it, -ray.d, ls.wo);
        f = it.material->F(mc) * AbsDot(ls.wo, it.n);
        if (!ls.isDelta)
            scatteringPdf = it.material->PDF(mc);
    } else {
        float p = it.phaseFunction->P(-ray.d, ls.wo);
        f = Spectrum(p);
        scatteringPdf = p;
    }
    float w = PowerHeuristic(1, ls.pdf, 1, scatteringPdf);

    if (ls.isDelta)
        return tr * f * ls.Le / ls.pdf;
    else
        return tr * f * w * ls.Le / ls.pdf;
}

void PixelIntegrator::Render() {
    Profiler _("Rendering");
    film->Clear();

    int total = Area(filmSize);
    int groupSize = max(total / 100, 1);
    int nGroups = (total + groupSize - 1) / groupSize;

    ProgressReporter pr("Rendering", "Pixels", "Samples", total, samplesPerPixel);

    for (int i = 0; i < nGroups; i++) {
        ScopedPR(pr, i * groupSize, i + 1 == nGroups);

        ParallelFor(groupSize, [&](int index) {
            index += i * groupSize;
            if (index >= total)
                return;
            vec2i p = {index % filmSize.x, index / filmSize.x};
            Sampler& sampler = samplers[threadIdx];
            sampler.StartPixel(p, 0);

            for (int sampleIndex = 0; sampleIndex < samplesPerPixel; sampleIndex++) {
                Compute(p, sampler);
                sampler.StartNextSample();
            }
        });
    }

    film->Finalize(1.0f / samplesPerPixel);
}

void RadianceIntegrator::Compute(vec2i p, Sampler& sampler) {
    vec2 pFilm = (p + sampler.Get2D()) / film->Size();
    Ray ray = scene->camera.GenRay(pFilm, sampler.Get2D());
    film->AddSample(pFilm, Li(ray, sampler));
}

}  // namespace pine