#ifndef PINE_IMPL_BVHVIZ_INTEGRATOR_H
#define PINE_IMPL_BVHVIZ_INTEGRATOR_H

#include <core/integrator.h>

namespace pine {

class BvhVizIntegrator : public PixelSampleIntegrator {
  public:
    using PixelSampleIntegrator::PixelSampleIntegrator;

    vec3 Li(Ray ray, RNG&) override {
        Interaction it;
        return Intersect(ray, it) ? ColorMap(it.bvh / 300.0f) : vec3(0.0f);
    }
};

}  // namespace pine

#endif  // PINE_IMPL_BVHVIZ_INTEGRATOR_H