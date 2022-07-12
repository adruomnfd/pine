#include <impl/integrator/area.h>

namespace pine {

Spectrum AreaIntegrator::Li(Ray ray, Sampler&) {
    return ray.d;
}

}  // namespace pine