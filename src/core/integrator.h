#ifndef PINE_CORE_INTEGRATOR_H
#define PINE_CORE_INTEGRATOR_H

#include <core/geometry.h>
#include <core/sampler.h>
#include <core/accel.h>
#include <core/image.h>
#include <util/parameters.h>

#include <memory>
#include <map>

namespace pine {

class Integrator {
  public:
    static std::shared_ptr<Integrator> Create(const Parameters& parameters);

    Integrator(const Parameters& parameters);
    virtual ~Integrator() {
        for (auto& sampler : samplers)
            Sampler::Destory(sampler);
    }
    PINE_DELETE_COPY_MOVE(Integrator)

    virtual void Initialize(const Scene* scene);
    virtual void Render() = 0;

  protected:
    const Scene* scene;

    Image film;
    vec2i filmSize;
    std::string outputFileName;

    std::vector<Sampler> samplers;
    int samplesPerPixel;
};

class RayIntegrator : public Integrator {
  public:
    RayIntegrator(const Parameters& parameters);
    virtual void Initialize(const Scene* scene) override;

    bool Hit(Ray ray);
    bool Intersect(Ray& ray, Interaction& it);
    bool IntersectTr(Ray ray, vec3& tr, Sampler& sampler);
    vec3 EstimateDirect(Ray ray, Interaction it, Sampler& sampler);

    std::vector<std::shared_ptr<Accel>> accels;
    Parameters parameters;
};

class PixelSampleIntegrator : public RayIntegrator {
  public:
    PixelSampleIntegrator(const Parameters& parameters) : RayIntegrator(parameters){};

    virtual void Render();
    virtual vec3 Li(Ray ray, Sampler& sampler) = 0;
};

class SinglePassIntegrator : public RayIntegrator {
  public:
    SinglePassIntegrator(const Parameters& parameters) : RayIntegrator(parameters){};

    virtual void Render();
    virtual vec3 Li(Ray ray, Sampler& sampler) = 0;
};

}  // namespace pine

#endif  // PINE_CORE_INTEGRATOR_H