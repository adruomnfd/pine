#include <impl/integrator/lightpath.h>
#include <core/scene.h>

namespace pine {

void LightPathIntegrator::Compute(vec2i, Sampler& sampler) {
    auto [light, lightPdf] = lightSampler.SampleLight(sampler.Get1D());
    if (!light)
        return;

    auto les = light->SampleLe(sampler.Get2D(), sampler.Get2D());

    Ray ray = les.ray;
    Spectrum beta = les.Le / (lightPdf * les.pdf);

    for (int depth = 1; depth < maxDepth; depth++) {
        Interaction it;
        if (!Intersect(ray, it))
            continue;

        if (!it.material) {
            ray = it.SpawnRay(ray.d);
            --depth;
            continue;
        }

        auto cs = scene->camera.SampleWi(it.p, sampler.Get2D());
        it.n = it.material->BumpNormal(MaterialEvalCtx(it, -ray.d));
        auto mc = MaterialEvalCtx(it, -ray.d, cs.wo);

        if (!cs.we.IsBlack() && !Hit(it.SpawnRayTo(cs.p))) {
            Spectrum f = it.material->F(mc);
            Spectrum L = beta * f * AbsDot(cs.wo, it.n) * cs.we / cs.pdf;
            film->AddSplat(cs.pFilm, L);
        }

        if (depth + 1 == maxDepth)
            break;

        mc.u1 = sampler.Get1D();
        mc.u2 = sampler.Get2D();
        if (auto bs = it.material->Sample(mc)) {
            beta *= AbsDot(bs->wo, it.n) * bs->f / bs->pdf;
            ray = it.SpawnRay(bs->wo);
        } else {
            break;
        }
    }
}

}  // namespace pine