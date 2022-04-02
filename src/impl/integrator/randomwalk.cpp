#include <impl/integrator/randomwalk.h>
#include <core/scene.h>

namespace pine {

Spectrum RandomWalkIntegrator::Li(Ray ray, Sampler& sampler) {
    SampledProfiler _(ProfilePhase::EstimateLi);
    Spectrum L(0.0f), beta(1.0f);

    for (int depth = 0; depth < maxDepth; depth++) {
        Interaction it;
        bool foundIntersection = Intersect(ray, it);

        Interaction mi;
        if (ray.medium)
            beta *= ray.medium->Sample(ray, mi, sampler);

        if (mi.IsMediumInteraction()) {
            if (depth + 1 == maxDepth)
                break;
            vec3 wo;
            mi.phaseFunction->Sample(-ray.d, wo, sampler.Get2D());
            ray = mi.SpawnRay(wo);
            continue;
        }

        if (!foundIntersection) {
            if (scene->envLight) {
                Spectrum le = scene->envLight->Color(ray.d);
                L += beta * le;
            }
            break;
        }

        if (!it.material) {
            ray = it.SpawnRay(ray.d);
            --depth;
            continue;
        }

        it.n = it.material->BumpNormal(MaterialEvalCtx(it, -ray.d));
        auto mc = MaterialEvalCtx(it, -ray.d);

        if (it.material->Is<EmissiveMaterial>()) {
            Spectrum le = it.material->Le(mc);
            L += beta * le;
            break;
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

    return L;
}

}  // namespace pine