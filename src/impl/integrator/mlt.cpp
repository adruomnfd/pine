#include <impl/integrator/mlt.h>
#include <impl/integrator/path.h>
#include <impl/integrator/bdpt.h>
#include <impl/integrator/randomwalk.h>
#include <core/scene.h>

namespace pine {

MltIntegrator::MltIntegrator(const Parameters& params, Scene* scene) : Integrator(params, scene) {
    std::string baseMethod = params.GetString("baseMethod", "path");
    SWITCH(baseMethod) {
        CASE("path") integrator = std::make_unique<PathIntegrator>(params, scene);
        CASE("bdpt") bdpt = std::make_unique<BDPTIntegrator>(params, scene);
        CASE("randomwalk") integrator = std::make_unique<RandomWalkIntegrator>(params, scene);
        DEFAULT {
            LOG_FATAL("[MltIntegrator]Unsupported base method &", baseMethod);
            integrator = std::make_unique<PathIntegrator>(params, scene);
        }
    }
    nMutations = (int64_t)Area(filmSize) * params.GetInt("mutationsPerPixel", samplesPerPixel);
};

static const int cameraStreamIndex = 0;
static const int lightStreamIndex = 1;
static const int connectionStreamIndex = 2;
static const int nSampleStreams = 3;

Spectrum MltIntegrator::L(Sampler& sampler, int depth, vec2& pFilm) {
    sampler.Be<MltSampler>().StartStream(cameraStreamIndex);
    int s, t, nStrategies;
    if (depth == 0) {
        nStrategies = 1;
        s = 0;
        t = 2;
    } else {
        nStrategies = depth + 2;
        s = sampler.Get1D() * nStrategies;
        t = nStrategies - s;
    }

    Vertex* cameraVertices = &bdpt->cameraVertices[threadIdx][0];
    Vertex* lightVertices = &bdpt->lightVertices[threadIdx][0];

    pFilm = sampler.Get2D();

    if (GenerateCameraSubpath(scene, *bdpt, sampler, t, pFilm, cameraVertices) != t)
        return Spectrum(0.0f);

    sampler.Be<MltSampler>().StartStream(lightStreamIndex);
    if (GenerateLightSubpath(scene, *bdpt, sampler, s, lightVertices) != s)
        return Spectrum(0.0f);

    sampler.Be<MltSampler>().StartStream(connectionStreamIndex);

    return ConnectBDPT(*bdpt, lightVertices, cameraVertices, s, t, scene->camera, sampler, pFilm) *
           nStrategies;
}

void MltIntegrator::Render() {
    int64_t nMarkovChains = NumThreads() * 32;
    int64_t nMutationsPerChain = std::max(nMutations / nMarkovChains, 1l);
    int64_t nBootstrapSamples = nMutations / 32;

    ProgressReporter pr("Rendering", "MarkovChains", "Mutations", nMarkovChains,
                        nMutationsPerChain);

    Timer timer;
    AtomicFloat atomicI;
    ParallelFor(nBootstrapSamples, [&](int index) {
        if (bdpt) {
            for (int depth = 0; depth < bdpt->maxDepth; depth++) {
                Sampler sampler =
                    MltSampler(0.01f, 0.3f, nSampleStreams, index * bdpt->maxDepth + depth);
                vec2 pFilm;
                Spectrum l = MltIntegrator::L(sampler, depth, pFilm);
                if (!l.HasInfs() && !l.HasNaNs())
                    atomicI.Add(l.y());
            }
        } else {
            Sampler sampler = UniformSampler(1, index);
            vec2 pFilm = sampler.Get2D();
            Ray ray = scene->camera.GenRay(pFilm, sampler.Get2D());
            atomicI.Add(integrator->Li(ray, sampler).y());
        }
    });
    float I = (float)atomicI / nBootstrapSamples;

    ParallelFor(nMarkovChains, [&](int chainIndex) {
        ScopedPR(pr, chainIndex, chainIndex == nMarkovChains - 1, threadIdx == 0);
        RNG rng(chainIndex);
        Sampler sampler = MltSampler(0.01f, 0.3f, bdpt ? nSampleStreams : 1, chainIndex);

        auto L = [&]() {
            if (bdpt) {
                vec2 pFilm;
                Spectrum l = MltIntegrator::L(sampler, rng.Uniformf() * bdpt->maxDepth, pFilm);
                return std::pair{pFilm, l};
            } else {
                vec2 pFilm = sampler.Get2D();
                Ray ray = scene->camera.GenRay(pFilm, sampler.Get2D());
                return std::pair(pFilm, integrator->Li(ray, sampler));
            }
        };

        auto [pFilmCurrent, Lcurrent] = L();

        for (int m = 0; m < nMutationsPerChain; m++) {
            sampler.StartNextSample();
            auto [pFilmProposed, Lproposed] = L();
            float pAccept = Clamp(Lproposed.y() / Lcurrent.y(), 0.0f, 1.0f);
            if (Lcurrent.HasInfs() || Lcurrent.HasNaNs())
                pAccept = 1.0f;

            float pdfCurrent = Lcurrent.y() / I;
            float pdfProposed = Lproposed.y() / I;
            Spectrum wCurrent = (1.0f - pAccept) * Lcurrent * SafeRcp(pdfCurrent);
            Spectrum wProposed = pAccept * Lproposed * SafeRcp(pdfProposed);

            if (!wCurrent.HasInfs() && !wCurrent.HasNaNs())
                film->AddSplat(pFilmCurrent, wCurrent);
            if (!wProposed.HasInfs() && !wProposed.HasNaNs())
                film->AddSplat(pFilmProposed, wProposed);

            if (rng.Uniformf() < pAccept) {
                pFilmCurrent = pFilmProposed;
                Lcurrent = Lproposed;
                sampler.Be<MltSampler>().Accept();
            } else {
                sampler.Be<MltSampler>().Reject();
            }
        }
    });

    double dA = 1.0f / Area(filmSize);
    double N = nMutations;
    film->Finalize(1.0 / dA / N);
}

}  // namespace pine