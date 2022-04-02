#ifndef PINE_IMPL_INTEGRATOR_LIGHTPATH_H
#define PINE_IMPL_INTEGRATOR_LIGHTPATH_H

#include <core/integrator.h>

namespace pine {

class LightPathIntegrator : public PixelIntegrator {
  public:
    using PixelIntegrator::PixelIntegrator;
    void Compute(vec2i p, Sampler& sampler) override;
};

}  // namespace pine

#endif  // PINE_IMPL_INTEGRATOR_LIGHTPATH_H