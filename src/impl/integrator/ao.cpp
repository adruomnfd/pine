#include <impl/integrator/ao.h>
#include <core/scene.h>
#include <core/color.h>
#include <util/parallel.h>
#include <util/fileio.h>

namespace pine {

Spectrum AOIntegrator::Li(Ray ray, Sampler& sampler) {
    SampledProfiler _(ProfilePhase::EstimateLi);

    Interaction it;
    if (Intersect(ray, it)) {
        ray.o = it.p;
        it.n = FaceSameHemisphere(it.n, -ray.d);
        ray = it.SpawnRay(FaceSameHemisphere(UniformHemisphereMampling(sampler.Get2D()), it.n));
        return Hit(ray) ? Spectrum(0.0f) : Spectrum(1.0f);
    }
    return Spectrum(0.0f);
}

}  // namespace pine