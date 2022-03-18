#ifndef PINE_CORE_CAMERA_H
#define PINE_CORE_CAMERA_H

#include <core/geometry.h>
#include <core/medium.h>
#include <util/taggedptr.h>
#include <util/profiler.h>

namespace pine {

struct PinHoleCamera {
    static PinHoleCamera Create(const Parameters& params);

    PinHoleCamera() = default;
    PinHoleCamera(vec3 from, vec3 to, float fov)
        : origin(from), lookat(LookAt(from, to)), fovT(1 / std::tan(fov / 2)){};

    Ray GenRay(vec2 co, vec2) const;

    vec3 origin;
    mat3 lookat;
    float fovT;
};

struct ThinLenCamera {
    static ThinLenCamera Create(const Parameters& params);

    ThinLenCamera() = default;
    ThinLenCamera(vec3 from, vec3 to, float fov, float lenRadius, float focusDistance)
        : origin(from),
          lookat(LookAt(from, to)),
          fovT(1 / std::tan(fov / 2)),
          lenRadius(lenRadius),
          focusDistance(focusDistance){};

    Ray GenRay(vec2 co, vec2 u2) const;

    vec3 origin;
    mat3 lookat;
    float fovT;
    float lenRadius;
    float focusDistance;
};

struct Camera : public TaggedPointer<PinHoleCamera, ThinLenCamera> {
    using TaggedPointer::TaggedPointer;
    static Camera Create(const Parameters& params, Scene* scene);
    static void Destory(Camera& camera) {
        camera.Delete();
    }

    Ray GenRay(vec2 co, vec2 u2) const {
        SampledProfiler _(ProfilePhase::GenerateRay);
        Ray ray = Dispatch([&](auto ptr) { return ptr->GenRay(co, u2); });
        ray.medium = medium;
        return ray;
    }

    Medium medium;
};

}  // namespace pine

#endif  // PINE_CORE_CAMERA_H