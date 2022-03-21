#ifndef PINE_CORE_INTEGRATOR_H
#define PINE_CORE_INTEGRATOR_H

#include <core/lightsampler.h>
#include <core/geometry.h>
#include <core/sampler.h>
#include <core/accel.h>
#include <core/film.h>
#include <util/parameters.h>

#include <memory>
#include <map>

namespace pine {

class Integrator {
  public:
    static std::shared_ptr<Integrator> Create(const Parameters& parameters, const Scene* scene);
    Integrator(const Parameters& parameters, const Scene* scene);
    virtual ~Integrator() = default;

    virtual void Render() = 0;

  protected:
    const Scene* scene;

    Film film;
    vec2i filmSize;

    std::vector<Sampler> samplers;
    int samplesPerPixel;
    LightSampler lightSampler;
};

class RayIntegrator : public Integrator {
  public:
    RayIntegrator(const Parameters& parameters, const Scene* scene);

    bool Hit(Ray ray);
    bool Intersect(Ray& ray, Interaction& it);
    bool IntersectTr(Ray ray, Spectrum& tr, Sampler& sampler);
    Spectrum EstimateDirect(Ray ray, Interaction it, Sampler& sampler);

    std::shared_ptr<Accel> accel;
    std::vector<int> meshIndices;
};

class PixelSampleIntegrator : public RayIntegrator {
  public:
    using RayIntegrator::RayIntegrator;

    virtual void Render();
    virtual Spectrum Li(Ray ray, Sampler& sampler) = 0;
};

class SinglePassIntegrator : public RayIntegrator {
  public:
    using RayIntegrator::RayIntegrator;

    virtual void Render();
    virtual Spectrum Li(Ray ray, Sampler& sampler) = 0;
};

}  // namespace pine

#endif  // PINE_CORE_INTEGRATOR_H