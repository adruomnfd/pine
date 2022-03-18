#ifndef PINE_IMPL_PATH_INTEGRATOR_H
#define PINE_IMPL_PATH_INTEGRATOR_H

#include <core/integrator.h>

namespace pine {

struct PathIntegrator : SinglePassIntegrator {
    PathIntegrator(const Parameters& parameters);
    std::optional<Spectrum> Li(Ray ray, Sampler& sampler) override;

    int maxDepth;
    float clamp;
};

}  // namespace pine

#endif  // PINE_IMPL_PATH_INTEGRATOR_H