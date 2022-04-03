#include <impl/integrator/path.h>
#include <core/scene.h>

namespace pine {

Spectrum PathIntegrator::Li(Ray ray, Sampler& sampler) {
    SampledProfiler _(ProfilePhase::EstimateLi);
    Spectrum L(0.0f), beta(1.0f);
    float bsdfPdf = 0.0f;

    for (int depth = 0; depth < maxDepth; depth++) {
        Interaction it;
        bool foundIntersection = Intersect(ray, it);

        Interaction mi;
        if (ray.medium)
            beta *= ray.medium->Sample(ray, mi, sampler);

        if (mi.IsMediumInteraction()) {
            if (depth + 1 == maxDepth)
                break;
            L += beta * EstimateDirect(ray, mi, sampler);
            vec3 wo;
            bsdfPdf = mi.phaseFunction->Sample(-ray.d, wo, sampler.Get2D());
            ray = mi.SpawnRay(wo);
            continue;
        }

        if (!foundIntersection) {
            if (scene->envLight) {
                Spectrum le = scene->envLight->Color(ray.d);
                if (depth == 0) {
                    L += beta * le;
                } else {
                    float lightPdf = scene->envLight->Pdf(ray.d);
                    L += beta * le * PowerHeuristic(1, bsdfPdf, 1, lightPdf);
                }
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
            if (depth == 0) {
                L += beta * le;
            } else {
                float lightPdf = it.shape->Pdf(ray, it);
                L += beta * le * PowerHeuristic(1, bsdfPdf, 1, lightPdf);
            }
            break;
        }

        if (depth + 1 == maxDepth)
            break;
            
        L += beta * EstimateDirect(ray, it, sampler);

        mc.u1 = sampler.Get1D();
        mc.u2 = sampler.Get2D();
        if (auto bs = it.material->Sample(mc)) {
            beta *= AbsDot(bs->wo, it.n) * bs->f / bs->pdf;
            bsdfPdf = bs->pdf;
            ray = it.SpawnRay(bs->wo);

            if (depth > 2) {
                float q = Clamp(1.0f - beta.y(), 0.05f, 1.0f);
                if (sampler.Get1D() < q)
                    break;
                else
                    beta /= 1.0f - q;
            }
        } else {
            break;
        }
    }
    
    return L;
}

}  // namespace pine