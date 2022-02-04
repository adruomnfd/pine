#ifndef PINE_CORE_MEDIUM_H
#define PINE_CORE_MEDIUM_H

#include <core/vecmath.h>
#include <util/taggedptr.h>
#include <util/profiler.h>

namespace pine {

float PhaseHG(float cosTheta, float g);

struct PhaseFunction {
    PhaseFunction() = default;
    PhaseFunction(float g) : g(g){};
    float F(vec3 wi, vec3 wo) const;
    float Sample(vec3 wi, vec3& wo, vec2 u2) const;

    float g = 0.0f;
};

struct MediumSample {
    vec3 tr = vec3(1.0f);
    vec3 wo;
};

struct HomogeneousMedium {
    static HomogeneousMedium Create(const Parameters& params);
    HomogeneousMedium(vec3 sigma_a, vec3 sigma_s, float g)
        : sigma_a(sigma_a), sigma_s(sigma_s), sigma_t(sigma_a + sigma_s), g(g){};

    vec3 Tr(const Ray& ray, RNG& rng) const;
    MediumSample Sample(const Ray& ray, Interaction& mi, RNG& rng) const;

    vec3 sigma_a;
    vec3 sigma_s;
    vec3 sigma_t;
    float g;
};

struct GridMedium {
    static GridMedium Create(const Parameters& params);
    GridMedium(vec3 sigma_a, vec3 sigma_s, float g, vec3i size, vec3 lower, vec3 upper,
               float* density);

    vec3 Tr(const Ray& ray, RNG& rng) const;
    MediumSample Sample(const Ray& ray, Interaction& mi, RNG& rng) const;
    float Density(vec3 p) const;
    float D(vec3i pi) const;

    vec3 sigma_a;
    vec3 sigma_s;
    float sigma_t;
    float g;
    vec3i size;
    float invMaxDensity;
    vec3 lower;
    vec3 upper;
    float* density;
};

struct Medium : TaggedPointer<HomogeneousMedium, GridMedium> {
    using TaggedPointer::TaggedPointer;
    static Medium Create(const Parameters& params);
    static void Destory(Medium medium) {
        medium.Delete();
    }

    vec3 Tr(const Ray& ray, RNG& rng) const {
        SampledProfiler _(ProfilePhase::MediumTr);
        return Dispatch([&](auto ptr) { return ptr->Tr(ray, rng); });
    }
    MediumSample Sample(const Ray& ray, Interaction& mi, RNG& rng) const {
        SampledProfiler _(ProfilePhase::MediumSample);
        return Dispatch([&](auto ptr) { return ptr->Sample(ray, mi, rng); });
    }
};

struct MediumInterface {
    MediumInterface() = default;
    MediumInterface(Medium medium) : inside(medium), outside(medium){};
    MediumInterface(Medium inside, Medium outside) : inside(inside), outside(outside){};
    bool IsMediumTransition() const {
        return inside != outside;
    }

    Medium inside;
    Medium outside;
};

}  // namespace pine

#endif  // PINE_CORE_MEDIUM_H