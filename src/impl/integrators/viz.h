#ifndef PINE_IMPL_BVHVIZ_INTEGRATOR_H
#define PINE_IMPL_BVHVIZ_INTEGRATOR_H

#include <core/integrator.h>

namespace pine {

class VizIntegrator : public PixelSampleIntegrator {
  public:
    VizIntegrator(const Parameters& parameters);

    std::optional<Spectrum> Li(Ray ray, Sampler&) override;

    enum class Type { Bvh, Position, Normal, Texcoord } type;
};

}  // namespace pine

#endif  // PINE_IMPL_BVHVIZ_INTEGRATOR_H