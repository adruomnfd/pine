#include <impl/integrator/mlt.h>
#include <core/scene.h>

namespace pine {

MltIntegrator::MltIntegrator(const Parameters& parameters, const Scene* scene)
    : PathIntegrator(parameters, scene) {
    nMutations = (int64_t)Area(filmSize) * parameters.GetInt("mutationsPerPixel", samplesPerPixel);
};

void MltIntegrator::Render() {
    int64_t nMarkovChains = NumThreads() * 16;
    int64_t nMutationsPerChain = std::max(nMutations / nMarkovChains, 1l);
    int64_t nBootstrapSamples = nMutations / 32;

    ProgressReporter pr("Rendering", "MarkovChains", "Mutations", nMarkovChains,
                        nMutationsPerChain);

    Timer timer;
    AtomicFloat atomicI;
    ParallelFor(nBootstrapSamples, [&](int index) {
        Sampler sampler = UniformSampler(1, index);
        vec2 pFilm = sampler.Get2D() * filmSize;
        Ray ray = scene->camera.GenRay(filmSize, pFilm, sampler.Get2D());
        atomicI.Add(Li(ray, sampler).y());
    });
    float I = (float)atomicI / nBootstrapSamples;

    ThreadIdParallelFor(nMarkovChains, [&](int threadId, int chainIndex) {
        ScopedPR(pr, chainIndex, chainIndex == nMarkovChains - 1, threadId == 0);
        RNG rng(chainIndex);
        Sampler sampler = MltSampler(0.01f, 0.3f, chainIndex);

        auto L = [&]() {
            vec2 pFilm = sampler.Get2D() * filmSize;
            Ray ray = scene->camera.GenRay(filmSize, pFilm, sampler.Get2D());
            return std::pair(pFilm, Li(ray, sampler));
        };

        auto [pFilmCurrent, Lcurrent] = L();

        for (int m = 0; m < nMutationsPerChain; m++) {
            sampler.StartNextSample();
            auto [pFilmProposed, Lproposed] = L();
            float pAccept = Clamp(Lproposed.y() / Lcurrent.y(), 0.0f, 1.0f);

            Spectrum wCurrent = (1.0f - pAccept) * Lcurrent * SafeRcp(Lcurrent.y());
            Spectrum wProposed = pAccept * Lproposed * SafeRcp(Lproposed.y());
            film.AddSample(pFilmCurrent, wCurrent);
            film.AddSample(pFilmProposed, wProposed);

            if (rng.Uniformf() < pAccept) {
                pFilmCurrent = pFilmProposed;
                Lcurrent = Lproposed;
                sampler.Be<MltSampler>().Accept();
            } else {
                sampler.Be<MltSampler>().Reject();
            }
        }
    });

    film.Finalize(I * (float)Area(filmSize) / nMutations, true);
}

}  // namespace pine