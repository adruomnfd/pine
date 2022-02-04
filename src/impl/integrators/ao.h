#ifndef PINE_IMPL_AO_INTEGRATOR_H
#define PINE_IMPL_AO_INTEGRATOR_H

#include <core/integrator.h>

namespace pine {

class AOIntegrator : public RayIntegrator {
  public:
    AOIntegrator(const Parameters& parameters) : RayIntegrator(parameters){};

    void Render() override;
    bool NextIteration() override;
};

}  // namespace pine

#endif  // PINE_IMPL_AO_INTEGRATOR_H