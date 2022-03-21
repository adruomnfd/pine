#include <impl/integrator/path.h>
#include <core/scene.h>
#include <core/color.h>
#include <util/parameters.h>

namespace pine {

PathIntegrator::PathIntegrator(const Parameters& parameters, const Scene* scene)
    : SinglePassIntegrator(parameters, scene) {
    maxDepth = parameters.GetInt("maxDepth", 4);
    clamp = parameters.GetFloat("clamp", FloatMax);
}

Spectrum PathIntegrator::Li(Ray ray, Sampler& sampler) {
    SampledProfiler _(ProfilePhase::EstimateLi);
    Spectrum L(0.0f), beta(1.0f);
    float bsdfPdf = 0.0f;

    for (int depth = 0; depth < maxDepth; depth++) {
        Interaction it;
        bool foundIntersection = Intersect(ray, it);

        // Sample medium scatter event
        Interaction mi;
        MediumSample ms;
        if (ray.medium) {
            ms = ray.medium->Sample(ray, mi, sampler);
            beta *= ms.tr;
        }

        // If medium scatter event happens
        if (mi.IsMediumInteraction()) {
            L += beta * EstimateDirect(ray, mi, sampler);
            ray = mi.SpawnRay(ms.wo);
            continue;
        }

        // If no medium scatter event happens and ray does not intersect with surface
        if (!mi.IsMediumInteraction() && !foundIntersection) {
            if (depth == 0 && !scene->envLight)
                return Spectrum(0.0f);
            if (scene->envLight)
                L += beta * scene->envLight->Color(ray.d);
            break;
        }

        // Handle medium boundary
        if (!it.material) {
            ray = it.SpawnRay(ray.d);
            depth--;
            continue;
        }

        MaterialEvalContext mc(it.p, it.n, it.uv, it.dpdu, it.dpdv, -ray.d);
        it.n = it.material->BumpNormal(mc);
        mc = MaterialEvalContext(it.p, it.n, it.uv, it.dpdu, it.dpdv, -ray.d);

        // Accounting for visible emssive surface
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

        if (depth == maxDepth - 1)
            break;

        // Sampling light
        L += beta * EstimateDirect(ray, it, sampler);

        // Sample next path
        mc.u1 = sampler.Get1D();
        mc.u2 = sampler.Get2D();
        if (auto bs = it.material->Sample(mc)) {
            beta *= AbsDot(bs->wo, it.n) * bs->f / bs->pdf;
            bsdfPdf = bs->pdf;
            ray = it.SpawnRay(bs->wo);

            if (depth > 2) {
                float terminatePdf = Clamp(1.0f - beta.y(), 0.0f, 1.0f);
                if (sampler.Get1D() < terminatePdf)
                    break;
                else
                    beta /= 1.0f - terminatePdf;
            }
        } else {
            // Break if failed to generate next path
            break;
        }
    }

    return Clamp(L, 0, clamp);
}

}  // namespace pine