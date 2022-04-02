#ifndef PINE_IMPL_INTEGRATOR_PATH_H
#define PINE_IMPL_INTEGRATOR_PATH_H

#include <core/integrator.h>

namespace pine {

struct PathIntegrator : RadianceIntegrator {
    using RadianceIntegrator::RadianceIntegrator;
    Spectrum Li(Ray ray, Sampler& sampler) override;
};

}  // namespace pine

#endif  // PINE_IMPL_INTEGRATOR_PATH_H