#ifndef PINE_IMPL_INTEGRATOR_LIGHTPATH_H
#define PINE_IMPL_INTEGRATOR_LIGHTPATH_H

#include <core/integrator.h>

namespace pine {

class LightPathIntegrator : public RayIntegrator {
  public:
    LightPathIntegrator(const Parameters& params, Scene* scene);
    void Render();

  private:
    int maxDepth = 0;
};

}  // namespace pine

#endif  // PINE_IMPL_INTEGRATOR_LIGHTPATH_H