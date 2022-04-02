#ifndef PINE_IMPL_INTEGRATOR_PATH_H
#define PINE_IMPL_INTEGRATOR_PATH_H

#include <core/integrator.h>

namespace pine {

struct PathIntegrator : RadianceIntegrator {
    PathIntegrator(const Parameters& params, Scene* scene);
    Spectrum Li(Ray ray, Sampler& sampler) override;

    float clamp;
};

}  // namespace pine

#endif  // PINE_IMPL_INTEGRATOR_PATH_H