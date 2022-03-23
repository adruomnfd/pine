#ifndef PINE_CORE_CAMERA_H
#define PINE_CORE_CAMERA_H

#include <core/geometry.h>
#include <core/medium.h>
#include <util/taggedvariant.h>
#include <util/profiler.h>

namespace pine {

struct PinHoleCamera {
    static PinHoleCamera Create(const Parameters& params);

    PinHoleCamera(vec3 from, vec3 to, float fov)
        : origin(from), lookat(LookAt(from, to)), fovT(1 / std::tan(fov / 2)){};

    Ray GenRay(vec2i filmSize, vec2 co, vec2 ul) const;
    Spectrum We(const Ray& ray);

    vec3 origin;
    mat3 lookat;
    float fovT;
};

struct ThinLenCamera {
    static ThinLenCamera Create(const Parameters& params);

    ThinLenCamera(vec3 from, vec3 to, float fov, float lenRadius, float focusDistance)
        : origin(from),
          lookat(LookAt(from, to)),
          fovT(1 / std::tan(fov / 2)),
          lenRadius(lenRadius),
          focusDistance(focusDistance){};

    Ray GenRay(vec2i filmSize, vec2 co, vec2 u2) const;

    vec3 origin;
    mat3 lookat;
    float fovT;
    float lenRadius;
    float focusDistance;
};

struct Camera : public TaggedVariant<PinHoleCamera, ThinLenCamera> {
    using TaggedVariant::TaggedVariant;
    static Camera Create(const Parameters& params, const Scene* scene);

    Ray GenRay(vec2i filmSize, vec2 co, vec2 u2) const {
        SampledProfiler _(ProfilePhase::GenerateRay);
        Ray ray = Dispatch([&](auto&& x) { return x.GenRay(filmSize, co, u2); });
        ray.medium = medium.get();
        return ray;
    }

    std::shared_ptr<Medium> medium;
};

}  // namespace pine

#endif  // PINE_CORE_CAMERA_H