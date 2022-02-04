#include <impl/integrators/photon.h>
#include <core/scene.h>
#include <core/color.h>
#include <util/parameters.h>
#include <util/parallel.h>
#include <util/fileio.h>

#include <algorithm>
#include <numeric>

namespace pine {

uint32_t PhotonMap::BuildNode(uint32_t* indices, uint32_t size, uint32_t depth) {
    uint32_t axis = depth % 3;
    std::sort(indices, indices + size,
              [&](int i0, int i1) { return photons[i0][axis] < photons[i1][axis]; });

    int mid = size / 2;
    uint32_t parentIndex = nodes.size();
    Node node;
    node.axis = axis;
    node.index = indices[mid];
    nodes.push_back(node);

    if (mid != 0) {
        nodes[parentIndex].leftChildIndex = BuildNode(indices, mid, depth + 1);
        nodes[parentIndex].rightChildIndex = BuildNode(indices + mid, size - mid, depth + 1);
    }
    return parentIndex;
}
void PhotonMap::Build() {
    std::vector<uint32_t> indices(photons.size());
    std::iota(indices.begin(), indices.end(), 0);
    BuildNode(indices.data(), photons.size(), 0);
}
void PhotonMap::SearchNode(uint32_t nodeIndex, vec3 point, uint32_t k, Queue& queue) const {
    if (nodeIndex == uint32_t(-1))
        return;
    const Node& node = nodes[nodeIndex];
    const Photon& median = photons[node.index];

    queue.emplace(node.index, Distance(point, median.position));
    if (queue.size() > k)
        queue.pop();

    bool isLower = point[node.axis] < median[node.axis];
    SearchNode(isLower ? node.leftChildIndex : node.rightChildIndex, point, k, queue);

    if (queue.top().distance > Sqr(median[node.axis] - point[node.axis]))
        SearchNode(isLower ? node.rightChildIndex : node.leftChildIndex, point, k, queue);
}
PhotonMap::Neighbors PhotonMap::SearchNeighbors(vec3 position, uint32_t numNeighbors) const {
    SampledProfiler _(ProfilePhase::SearchNeighbors);
    Queue queue;
    SearchNode(0, position, numNeighbors, queue);

    Neighbors collection;
    collection.neighbors.resize(queue.size());
    for (auto& neighbor : collection.neighbors) {
        neighbor = queue.top();
        collection.maxDistance = std::max(collection.maxDistance, neighbor.distance);
        queue.pop();
    }
    return collection;
}

PhotonIntegrator::PhotonIntegrator(const Parameters& parameters) : RayIntegrator(parameters) {
    maxDepth = parameters.GetInt("maxDepth", 4);
    visualizePhoton = parameters.GetBool("visualizePhoton", false);
    if (visualizePhoton)
        samplePerPixel = 1;
    numPhotons = parameters.GetInt("numPhotons", 10000);
    numPhotonsEstimation = parameters.GetInt("numPhotonsEstimation", 40);
}
void PhotonIntegrator::Render() {
    Profiler _("Render");
    Timer timer;
    RNG rng;

    for (int i = 0; i < numPhotons; i++) {
        LightEmissionSample les =
            scene->lights[rng.Uniform64u() % scene->lights.size()].SampleEmission(
                rng.Uniformf(), rng.Uniform2f(), rng.Uniform2f());
        Ray ray = Ray(les.p, les.wo);
        Interaction it;
        if (Intersect(ray, it)) {
            photonMap.AddPhoton(Photon(les.Le / scene->lights.size(), it.p, -les.wo));
        }
    }

    LOG("[PhotonMapper]Photons traced in & ms", timer.Reset());
    photonMap.Build();
    LOG("[PhotonMapper]Photon map built in & ms", timer.Reset());

    for (sampleIndex = 0; sampleIndex < samplePerPixel; sampleIndex++) {
        ParallelFor(filmSize, [&](vec2 p) {
            RNG rng(Hash(p, sampleIndex));
            Ray ray = scene->camera.GenRay(
                (p - filmSize / 2 + rng.Uniform2f() - vec2(0.5f)) / filmSize.y, rng.Uniform2f());

            vec3 Li;
            vec3 beta(1.0f);

            for (int depth = 0; depth < maxDepth; depth++) {
                Interaction it;
                if (!Intersect(ray, it))
                    break;

                MaterialEvalContext mc(it.p, it.n, it.uv, -ray.d);

                if (visualizePhoton ? depth == 0 : depth > 0)
                    Li += beta * Radiance(-ray.d, it);

                if (visualizePhoton ? false : depth == 0)
                    Li += EstimateDirect(ray, it, rng);

                // Sample next path
                mc.u1 = rng.Uniformf();
                mc.u2 = rng.Uniform2f();
                BSDFSample bs = it.material.Sample(mc);
                // Break if fail to generate next path
                if (bs.wo == vec3(0.0f))
                    return;
                beta *= AbsDot(bs.wo, it.n) * bs.f / bs.pdf;
                ray = it.SpawnRay(bs.wo);
            }

            film[p] += vec4(Li, 1.0f) / samplePerPixel;
        });
        pr.Report(sampleIndex);
    }
    LOG("[PhotonMapper]Render done in & ms", timer.Reset());

    ParallelFor(filmSize, [&](vec2i p) {
        film[p] = vec4(Pow(Uncharted2Flimic(film[p]), 1.0f / 2.2f), 1.0f);
    });
    SaveBMPImage(outputFileName, film.Size(), 4, (float*)film.Data());
}
bool PhotonIntegrator::NextIteration() {
    return false;
}
vec3 PhotonIntegrator::Radiance(vec3 wo, Interaction it) const {
    PhotonMap::Neighbors neighbors = photonMap.SearchNeighbors(it.p, numPhotonsEstimation);
    vec3 Lo;
    const float k = 1.5f;
    const float r = neighbors.maxDistance;
    for (auto& neighbor : neighbors.neighbors) {
        const Photon& photon = photonMap.GetPhoton(neighbor.photonIndex);

        MaterialEvalContext mc;
        mc.p = it.p;
        mc.n = it.n;
        mc.wi = photon.wi;
        mc.wo = wo;
        mc.uv = it.uv;
        vec3 f = it.material.F(mc);
        float weight = 1.0f - neighbor.distance / (k * r);
        Lo += f * photon.flux * weight;
    }
    Lo /= numPhotons * Pi * Sqr(neighbors.maxDistance) * (1.0f - 2.0f / (3.0f * k));
    return Lo;
}

}  // namespace pine