#ifndef PINE_IMPL_AO_INTEGRATOR_H
#define PINE_IMPL_AO_INTEGRATOR_H

#include <core/integrator.h>

namespace pine {

class AOIntegrator : public SinglePassIntegrator {
  public:
    AOIntegrator(const Parameters& parameters) : SinglePassIntegrator(parameters){};

    Spectrum Li(Ray ray, Sampler& sampler) override;
};

}  // namespace pine

#endif  // PINE_IMPL_AO_INTEGRATOR_H