#include <impl/integrator/lightpath.h>
#include <core/scene.h>

namespace pine {

LightPathIntegrator::LightPathIntegrator(const Parameters& params, Scene* scene)
    : RayIntegrator(params, scene) {
    maxDepth = params.GetInt("maxDepth", 4);
}

void LightPathIntegrator::Render() {
    film->Clear();

    ThreadIdParallelFor(filmSize, [&](int threadId, vec2i p) {
        Sampler& sampler = samplers[threadId];
        sampler.StartPixel(p, 0);

        for (int sample = 0; sample < samplesPerPixel; sample++) {
            sampler.StartNextSample();
            auto [light, slPdf] = lightSampler.SampleLight(sampler.Get1D());
            if (!light)
                continue;
            auto les = light->SampleLe(sampler.Get2D(), sampler.Get2D());
            Ray ray = les.ray;

            Spectrum beta(1.0f);
            for (int depth = 0; depth < maxDepth; depth++) {
                Interaction it;
                if (!Intersect(ray, it))
                    continue;

                auto cs = scene->camera.SampleWi(it.p, sampler.Get2D());

                MaterialEvalContext mc(it.p, it.n, it.uv, it.dpdu, it.dpdv, -ray.d, cs.wo,
                                       sampler.Get2D(), sampler.Get1D());
                Spectrum f = it.material->F(mc);

                if (!Hit(it.SpawnRayTo(cs.p))) {
                    Spectrum L = beta * les.Le * AbsDot(cs.wo, it.n) * f * cs.we / cs.pdf;
                    film->AddSample(cs.pFilm, L);
                }

                if (auto bs = it.material->Sample(mc)) {
                    beta *= AbsDot(bs->wo, it.n) * bs->f / bs->pdf;
                    ray = it.SpawnRay(bs->wo);
                } else {
                    break;
                }
            }
        }
    });

    film->Finalize();
}

}  // namespace pine