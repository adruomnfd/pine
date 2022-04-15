#ifndef PINE_CORE_INTERACTION_H
#define PINE_CORE_INTERACTION_H

#include <core/ray.h>

namespace pine {

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

struct Interaction {
    bool IsSurfaceInteraction() const {
        return phaseFunction == nullptr;
    }
    bool IsMediumInteraction() const {
        return phaseFunction != nullptr;
    }
    Ray SpawnRayTo(vec3 p2) const {
        Ray ray;
        ray.d = Normalize(p2 - p, ray.tmax);
        ray.o = OffsetRayOrigin(p, FaceSameHemisphere(n, ray.d));
        ray.tmin = 0.0f;
        ray.tmax *= 1.0f - 1e-3f;
        ray.medium = GetMedium(ray.d);
        return ray;
    }
    Ray SpawnRay(vec3 wo, float distance) const {
        Ray ray;
        ray.d = wo;
        ray.o = OffsetRayOrigin(p, FaceSameHemisphere(n, ray.d));
        ray.tmin = 0.0f;
        ray.tmax = distance * (1.0f - 1e-3f);
        ray.medium = GetMedium(ray.d);
        return ray;
    }
    Ray SpawnRay(vec3 w) const {
        Ray ray;
        ray.d = w;
        ray.o = OffsetRayOrigin(p, FaceSameHemisphere(n, ray.d));
        ray.tmin = 0.0f;
        ray.tmax = FloatMax;
        ray.medium = GetMedium(ray.d);
        return ray;
    }
    const Medium* GetMedium(vec3 w) const {
        return Dot(w, n) > 0 ? mediumInterface.outside : mediumInterface.inside;
    }

    vec3 p;
    vec3 n;
    vec2 uv;
    vec3 wi;
    vec3 dpdu, dpdv;

    const Material* material = nullptr;
    const Shape* shape = nullptr;
    MediumInterface<const Medium*> mediumInterface;
    const PhaseFunction* phaseFunction = nullptr;
    float bvh = 0.0f;
};

}  // namespace pine

#endif  // PINE_CORE_INTERACTION_H