#ifndef PINE_IMPL_INTEGRATOR_MLT_H
#define PINE_IMPL_INTEGRATOR_MLT_H

#include <core/integrator.h>
#include <impl/integrator/path.h>

namespace pine {

struct MltIntegrator : public PathIntegrator {
    MltIntegrator(const Parameters& parameters, const Scene* scene);

    void Render() override;

  private:
    int64_t nMutations = 0;
};

}  // namespace pine

#endif