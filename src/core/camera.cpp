#include <core/camera.h>
#include <core/scene.h>
#include <util/parameters.h>

namespace pine {

PinHoleCamera PinHoleCamera::Create(const Parameters& params) {
    return PinHoleCamera(params.GetVec3("from"), params.GetVec3("to"), params.GetFloat("fov"));
}

Ray PinHoleCamera::GenRay(vec2 co, vec2) const {
    Ray ray;
    ray.o = origin;
    ray.d = lookat * Normalize(vec3(co, fovT));
    return ray;
}

ThinLenCamera ThinLenCamera::Create(const Parameters& params) {
    return ThinLenCamera(params.GetVec3("from"), params.GetVec3("to"), params.GetFloat("fov"),
                         params.GetFloat("lenRadius"), params.GetFloat("focusDistance"));
}

Ray ThinLenCamera::GenRay(vec2 co, vec2 u2) const {
    Ray ray;
    vec3 dir = Normalize(vec3(co, fovT));
    vec3 at = origin + lookat * (focusDistance * dir / dir.z);
    vec3 jitter = lenRadius * (lookat * vec3(SampleDiskPolar(u2), 0.0f));
    ray.o = origin + jitter;
    ray.d = Normalize(at - ray.o);
    return ray;
}

Camera Camera::Create(const Parameters& params, Scene* scene) {
    Camera camera;
    std::string type = params.GetString("type");
    if (type == "PinHole") {
        camera = new PinHoleCamera(PinHoleCamera::Create(params));
    } else if (type == "ThinLen") {
        camera = new ThinLenCamera(ThinLenCamera::Create(params));
    } else {
        LOG_WARNING("[Camera][Create]Unknown type \"&\"", type);
        camera = new PinHoleCamera(PinHoleCamera::Create(params));
    }
    camera.medium = scene->mediums[params.GetString("medium")];
    if (camera.medium)
        LOG_VERBOSE("[Camera]In medium");
    return camera;
}

}  // namespace pine