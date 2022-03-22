#ifndef PINE_CORE_MEDIUM_H
#define PINE_CORE_MEDIUM_H

#include <core/vecmath.h>
#include <core/sampler.h>
#include <core/spectrum.h>
#include <util/taggedvariant.h>
#include <util/profiler.h>

namespace pine {

struct PhaseFunction {
    PhaseFunction() = default;
    PhaseFunction(float g) : g(g){};
    float P(vec3 wi, vec3 wo) const;
    float Sample(vec3 wi, vec3& wo, vec2 u2) const;

    float g = 0.0f;
};

struct HomogeneousMedium {
    static HomogeneousMedium Create(const Parameters& params);
    HomogeneousMedium(Spectrum sigma_a, Spectrum sigma_s, PhaseFunction phaseFunction)
        : sigma_a(sigma_a),
          sigma_s(sigma_s),
          sigma_t(sigma_a + sigma_s),
          phaseFunction(phaseFunction){};

    Spectrum Tr(const Ray& ray, Sampler& sampler) const;
    Spectrum Sample(const Ray& ray, Interaction& mi, Sampler& sampler) const;

    Spectrum sigma_a;
    Spectrum sigma_s;
    Spectrum sigma_t;
    PhaseFunction phaseFunction;
};

struct GridMedium {
    static GridMedium Create(const Parameters& params);
    GridMedium(Spectrum sigma_a, Spectrum sigma_s, PhaseFunction phaseFunction, vec3i size,
               vec3 position, float scale, std::vector<float> density, bool interpolate);

    Spectrum Tr(const Ray& ray, Sampler& sampler) const;
    Spectrum Sample(const Ray& ray, Interaction& mi, Sampler& sampler) const;
    float Density(vec3 p) const;
    float D(vec3i pi) const;

    Spectrum sigma_a;
    Spectrum sigma_s;
    float sigma_t;
    PhaseFunction phaseFunction;
    vec3i size;
    float invMaxDensity;
    mat4 m2w;
    mat4 w2m;
    std::vector<float> density;
    bool interpolate = false;
};

struct Medium : TaggedVariant<HomogeneousMedium, GridMedium> {
    using TaggedVariant::TaggedVariant;
    static Medium Create(const Parameters& params);

    Spectrum Tr(const Ray& ray, Sampler& sampler) const {
        SampledProfiler _(ProfilePhase::MediumTr);
        return Dispatch([&](auto&& x) { return x.Tr(ray, sampler); });
    }
    Spectrum Sample(const Ray& ray, Interaction& mi, Sampler& sampler) const {
        SampledProfiler _(ProfilePhase::MediumSample);
        return Dispatch([&](auto&& x) { return x.Sample(ray, mi, sampler); });
    }
};

template <typename T>
struct MediumInterface {
    MediumInterface() = default;
    MediumInterface(const T& medium) : inside(medium), outside(medium){};
    MediumInterface(const T& inside, const T& outside) : inside(inside), outside(outside){};
    bool IsMediumTransition() const {
        return inside != outside;
    }

    T inside = {}, outside = {};
};

}  // namespace pine

#endif  // PINE_CORE_MEDIUM_H