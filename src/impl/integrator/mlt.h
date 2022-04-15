#ifndef PINE_IMPL_INTEGRATOR_MLT_H
#define PINE_IMPL_INTEGRATOR_MLT_H

#include <core/integrator.h>

namespace pine {

struct MltIntegrator : public Integrator {
    MltIntegrator(const Parameters& params, Scene* scene);

    void Render() override;

  private:
    Spectrum L(Sampler& sampler, int depth, vec2& pFilm);

    pstd::unique_ptr<RadianceIntegrator> integrator;
    pstd::unique_ptr<class BDPTIntegrator> bdpt;
    int64_t nMutations = 0;

    float sigma = 0.0f;
    float largeStepProbability = 0.0f;
};

}  // namespace pine

#endif