#ifndef PINE_IMPL_INTEGRATOR_SPPM_H
#define PINE_IMPL_INTEGRATOR_SPPM_H

#include <core/integrator.h>

namespace pine {

class SPPMIntegrator : public RayIntegrator {
  public:
    SPPMIntegrator(const Parameters& params, Scene* scene);
    void Render() override;

  private:
    float initialSearchRadius = 0.0f;
    int nIterations = 0;
    int photonsPerIteration = 0;
    int writeFrequency = 0;
    int finalGatheringDepth = 0;
};
}  // namespace pine

#endif  // PINE_IMPL_INTEGRATOR_SPPM_H