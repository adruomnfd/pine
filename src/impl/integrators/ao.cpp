#include <impl/integrators/ao.h>
#include <core/scene.h>
#include <core/color.h>
#include <util/parallel.h>
#include <util/fileio.h>

namespace pine {

std::optional<Spectrum> AOIntegrator::Li(Ray ray, Sampler& sampler) {
    SampledProfiler _(ProfilePhase::EstimateLi);

    Interaction it;
    if (Intersect(ray, it)) {
        ray.o = it.p;
        it.n = FaceSameHemisphere(it.n, -ray.d);
        ray = it.SpawnRay(FaceSameHemisphere(UniformHemisphereSampling(sampler.Get2D()), it.n));
        return Intersect(ray, it) ? Spectrum(0.0f) : Spectrum(1.0f);
    }
    return Spectrum(0.0f);
}

}  // namespace pine