#include <impl/integrators/path.h>
#include <core/scene.h>
#include <core/color.h>
#include <util/parameters.h>

namespace pine {

PathIntegrator::PathIntegrator(const Parameters& parameters, const Scene* scene)
    : SinglePassIntegrator(parameters, scene) {
    maxDepth = parameters.GetInt("maxDepth", 4);
    clamp = parameters.GetFloat("clamp", FloatMax);
}
std::optional<Spectrum> PathIntegrator::Li(Ray ray, Sampler& sampler) {
    SampledProfiler _(ProfilePhase::EstimateLi);
    Spectrum L(0.0f);
    Spectrum beta(1.0f);
    float bsdfPDF;

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

        it.n = it.material->BumpNormal(MaterialEvalContext(it.p, it.n, it.uv, it.dpdu, it.dpdv, -ray.d));

        MaterialEvalContext mc(it.p, it.n, it.uv, it.dpdu, it.dpdv, -ray.d);

        // Accounting for visible emssive surface
        Spectrum le = it.material->Le(mc);
        if (!le.IsBlack()) {
            if (depth == 0)
                L += beta * le;
            break;
        }

        // Sampling light
        L += beta * EstimateDirect(ray, it, sampler);

        // Sample next path
        mc.u1 = sampler.Get1D();
        mc.u2 = sampler.Get2D();
        if (auto bs = it.material->Sample(mc)) {
            beta *= AbsDot(bs->wo, it.n) * bs->f / bs->pdf;
            bsdfPDF = bs->pdf;
            ray = it.SpawnRay(bs->wo);
        } else {
            // Break if fail to generate next path
            break;
        }
    }

    return Clamp(L, 0, clamp);
}

}  // namespace pine