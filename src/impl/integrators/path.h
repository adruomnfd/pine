#ifndef PINE_IMPL_PATH_INTEGRATOR_H
#define PINE_IMPL_PATH_INTEGRATOR_H

#include <core/integrator.h>

namespace pine {

struct PathIntegrator : PixelSampleIntegrator {
    PathIntegrator(const Parameters& parameters);
    virtual vec3 Li(Ray ray, RNG& rng);
    
    int maxDepth;
};

}  // namespace pine

#endif  // PINE_IMPL_PATH_INTEGRATOR_H