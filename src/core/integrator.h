#ifndef PINE_CORE_INTEGRATOR_H
#define PINE_CORE_INTEGRATOR_H

#include <core/geometry.h>
#include <core/accel.h>
#include <core/image.h>
#include <util/parameters.h>

#include <memory>
#include <map>

namespace pine {

struct AOV {
    const vec4* data = nullptr;
    vec2i size;

    size_t SizeInBytes() const {
        return sizeof(data[0]) * size.x * size.y;
    }
};

class Integrator {
  public:
    static std::shared_ptr<Integrator> Create(const Parameters& parameters);

    Integrator(const Parameters& parameters);
    virtual ~Integrator() = default;
    virtual void Initialize(const Scene* scene);
    virtual void Render() = 0;
    virtual bool NextIteration() = 0;
    AOV GetAOV(std::string name);

  protected:
    const Scene* scene;

    Image film;
    vec2i filmSize;
    std::string outputFileName;
    std::map<std::string, AOV> aovs;

    ProgressReporter pr;
    int sampleIndex = 0;
    int samplePerPixel;

    vec3 sunDirection;
    float sunIntensity;
};

class RayIntegrator : public Integrator {
  public:
    RayIntegrator(const Parameters& parameters);
    virtual void Initialize(const Scene* scene);

    bool Hit(Ray ray);
    bool Intersect(Ray& ray, Interaction& it);
    bool IntersectTr(Ray ray, vec3& tr, RNG& rng);
    vec3 EstimateDirect(Ray ray, Interaction it, RNG& rng);

    std::vector<std::shared_ptr<Accel>> accels;
    Parameters parameters;
};

class PixelSampleIntegrator : public RayIntegrator {
  public:
    PixelSampleIntegrator(const Parameters& parameters) : RayIntegrator(parameters){};
    virtual void Render();
    virtual bool NextIteration();
    virtual vec3 Li(Ray ray, RNG& rng) = 0;
};

}  // namespace pine

#endif  // PINE_CORE_INTEGRATOR_H