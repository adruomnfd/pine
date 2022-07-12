#ifndef PINE_IMPL_INTEGRATOR_RANDOMWALK_H
#define PINE_IMPL_INTEGRATOR_RANDOMWALK_H

#include <core/integrator.h>

namespace pine {

struct RandomWalkIntegrator : RadianceIntegrator {
    using RadianceIntegrator::RadianceIntegrator;
    Spectrum Li(Ray ray, Sampler& sampler) override;
};

}  // namespace pine

#endif  // PINE_IMPL_INTEGRATOR_RANDOMWALK_H