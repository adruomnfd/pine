#include <core/camera.h>
#include <core/scene.h>
#include <util/parameters.h>
#include <util/misc.h>

namespace pine {

PinHoleCamera PinHoleCamera::Create(const Parameters& params) {
    return PinHoleCamera(params.GetVec3("from"), params.GetVec3("to"), params.GetFloat("fov"));
}

Ray PinHoleCamera::GenRay(vec2i filmSize, vec2 co, vec2) const {
    Ray ray;
    co = (co - filmSize / 2) / filmSize.y;
    ray.o = origin;
    ray.d = lookat * Normalize(vec3(co, fovT));
    return ray;
}

ThinLenCamera ThinLenCamera::Create(const Parameters& params) {
    return ThinLenCamera(params.GetVec3("from"), params.GetVec3("to"), params.GetFloat("fov"),
                         params.GetFloat("lenRadius"), params.GetFloat("focusDistance"));
}

Ray ThinLenCamera::GenRay(vec2i filmSize, vec2 co, vec2 u2) const {
    Ray ray;
    co = (co - filmSize / 2) / filmSize.y;
    vec3 dir = Normalize(vec3(co, fovT));
    vec3 at = origin + lookat * (focusDistance * dir / dir.z);
    vec3 jitter = lenRadius * (lookat * vec3(SampleDiskPolar(u2), 0.0f));
    ray.o = origin + jitter;
    ray.d = Normalize(at - ray.o);
    return ray;
}

Camera Camera::Create(const Parameters& params, const Scene* scene) {
    Camera camera;
    std::string type = params.GetString("type");

    SWITCH(type) {
        CASE("PinHole") camera = PinHoleCamera(PinHoleCamera::Create(params));
        CASE("ThinLen") camera = ThinLenCamera(ThinLenCamera::Create(params));
        DEFAULT {
            LOG_WARNING("[Camera][Create]Unknown type \"&\"", type);
            camera = PinHoleCamera(PinHoleCamera::Create(params));
        }
    }

    if (auto name = params.TryGetString("medium")) {
        auto medium = Find(scene->mediums, *name);
        if (!medium)
            LOG_WARNING("[Camera][Create]Medium \"&\" is not found", name);
        camera.medium = *medium;
    }
    if (camera.medium)
        LOG_VERBOSE("[Camera]In medium");
    return camera;
}

}  // namespace pine