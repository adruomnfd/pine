#ifndef PINE_IMPL_INTEGRATOR_AO_H
#define PINE_IMPL_INTEGRATOR_AO_H

#include <core/integrator.h>

namespace pine {

class AOIntegrator : public SinglePassIntegrator {
  public:
    using SinglePassIntegrator::SinglePassIntegrator;

    Spectrum Li(Ray ray, Sampler& sampler) override;
};

}  // namespace pine

#endif  // PINE_IMPL_INTEGRATOR_AO_H