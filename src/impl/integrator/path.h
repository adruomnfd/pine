#ifndef PINE_IMPL_INTEGRATOR_PATH_H
#define PINE_IMPL_INTEGRATOR_PATH_H

#include <core/integrator.h>

namespace pine {

struct PathIntegrator : SinglePassIntegrator {
    PathIntegrator(const Parameters& parameters, const Scene* scene);
    Spectrum Li(Ray ray, Sampler& sampler) override;

    int maxDepth;
    float clamp;
};

}  // namespace pine

#endif  // PINE_IMPL_INTEGRATOR_PATH_H