#include <impl/integrators/path.h>
#include <core/scene.h>
#include <core/color.h>
#include <util/parameters.h>

namespace pine {

PathIntegrator::PathIntegrator(const Parameters& parameters) : SinglePassIntegrator(parameters) {
    maxDepth = parameters.GetInt("maxDepth", 4);
    clamp = parameters.GetFloat("clamp", FloatMax);
}
Spectrum PathIntegrator::Li(Ray ray, Sampler& sampler) {
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
            ms = ray.medium.Sample(ray, mi, sampler);
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
            // L += beta * AtmosphereColor(ray.d, sunDirection, sunIntensity);
            break;
        }

        // Handle medium boundary
        if (!it.material) {
            ray = it.SpawnRay(ray.d);
            depth--;
            continue;
        }

        MaterialEvalContext mc;
        mc.n = it.n;
        mc.p = it.p;
        mc.uv = it.uv;
        mc.wi = -ray.d;

        // Accounting for visible emssive surface
        Spectrum le = it.material.Le(mc);
        if (!le.IsBlack()) {
            if (depth == 0) {
                L += beta * le;
            } else {
                // float lightPdf = it.pdf / scene->lights.size();
                // L += beta * le * PowerHeuristic(1, bsdfPDF, 1, lightPdf);
            }
            break;
        }

        // Sampling light
        L += beta * EstimateDirect(ray, it, sampler);

        // Sample next path
        mc.u1 = sampler.Get1D();
        mc.u2 = sampler.Get2D();
        BSDFSample bs = it.material.Sample(mc);
        // Break if fail to generate next path
        if (bs.wo == vec3(0.0f))
            break;
        beta *= AbsDot(bs.wo, it.n) * bs.f / bs.pdf;
        bsdfPDF = bs.pdf;
        Ray next = it.SpawnRay(bs.wo);
        next.tmin *= ray.tmax;
        ray = next;
    }

    return Clamp(L, 0, clamp);
}

}  // namespace pine