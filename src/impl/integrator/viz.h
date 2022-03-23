#ifndef PINE_IMPL_INTEGRATOR_VIZ_H
#define PINE_IMPL_INTEGRATOR_VIZ_H

#include <core/integrator.h>

namespace pine {

class VizIntegrator : public PixelSampleIntegrator {
  public:
    VizIntegrator(const Parameters& parameters,  Scene* scene);

    Spectrum Li(Ray ray, Sampler&) override;

    enum class Type { Bvh, Position, Normal, Texcoord, DpDu, DpDv } type;
};

}  // namespace pine

#endif  // PINE_IMPL_INTEGRATOR_VIZ_H