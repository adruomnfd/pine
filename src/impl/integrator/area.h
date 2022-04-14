#ifndef PINE_IMPL_INTEGRATOR_AREA_H
#define PINE_IMPL_INTEGRATOR_AREA_H

#include <core/integrator.h>

namespace pine {

class AreaIntegrator : public RadianceIntegrator {
  public:
    using RadianceIntegrator::RadianceIntegrator;
    
    Spectrum Li(Ray ray, Sampler& sampler);
};

}  // namespace pine

#endif  // PINE_IMPL_INTEGRATOR_AREA_H