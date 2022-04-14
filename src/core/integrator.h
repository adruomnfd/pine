#ifndef PINE_CORE_INTEGRATOR_H
#define PINE_CORE_INTEGRATOR_H

#include <core/lightsampler.h>
#include <core/geometry.h>
#include <core/sampler.h>
#include <core/accel.h>
#include <core/film.h>

#include <memory>
#include <map>

namespace pine {

class Integrator {
  public:
    Integrator(const Parameters& params, Scene* scene);
    virtual ~Integrator() = default;

    virtual void Render() = 0;

    LightSampler lightSampler;

    Scene* scene = nullptr;
    Film* film = nullptr;
    vec2i filmSize;

    std::vector<Sampler> samplers;
    int samplesPerPixel;
};

class RayIntegrator : public Integrator {
  public:
    RayIntegrator(const Parameters& params, Scene* scene);

    bool Hit(Ray ray) const;
    bool Intersect(Ray& ray, Interaction& it) const;
    Spectrum IntersectTr(Ray ray, Sampler& sampler) const;
    Spectrum EstimateDirect(Ray ray, Interaction it, Sampler& sampler) const;

    std::shared_ptr<Accel> accel;
    int maxDepth;
};

class PixelIntegrator : public RayIntegrator {
  public:
    using RayIntegrator::RayIntegrator;

    void Render() override;
    virtual void Compute(vec2i p, Sampler& sampler) = 0;
};

class RadianceIntegrator : public PixelIntegrator {
  public:
    using PixelIntegrator::PixelIntegrator;

    void Compute(vec2i p, Sampler& sampler) override;
    virtual Spectrum Li(Ray ray, Sampler& sampler) = 0;
};

Integrator* CreateIntegrator(const Parameters& params, Scene* scene);

}  // namespace pine

#endif  // PINE_CORE_INTEGRATOR_H