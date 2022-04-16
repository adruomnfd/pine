#ifndef PINE_CORE_RAY_H
#define PINE_CORE_RAY_H

#include <core/vecmath.h>
#include <util/string.h>

#include <pstd/tuple.h>

namespace pine {

struct Ray {
    static Ray SpawnRay(vec3 p0, vec3 p1) {
        Ray ray;
        ray.o = p0;
        ray.d = Normalize(p1 - p0, ray.tmax);
        return ray;
    };

    Ray() = default;
    Ray(vec3 o, vec3 d, float tmin = 0.0f, float tmax = FloatMax)
        : o(o), d(d), tmin(tmin), tmax(tmax){};
    vec3 operator()(float t) const {
        return o + t * d;
    }

    PSTD_ARCHIVE(o, d, tmin, tmax)

    vec3 o;
    vec3 d;
    float tmin = 0.0f;
    float tmax = FloatMax;
    const Medium* medium = nullptr;
};

inline vec3 OffsetRayOrigin(vec3 p, vec3 n) {
    float origin = 1.0f / 32.0f;
    float floatScale = 1.0f / 65536.0f;
    float intScale = 256.0f;
    vec3i of_i = intScale * n;
    vec3 p_i = pstd::bitcast<vec3>(pstd::bitcast<vec3i>(p) + vec3i(p.x < 0 ? -of_i.x : of_i.x,
                                                                   p.y < 0 ? -of_i.y : of_i.y,
                                                                   p.z < 0 ? -of_i.z : of_i.z));
    return {pstd::abs(p.x) < origin ? p.x + n.x * floatScale : p_i.x,
            pstd::abs(p.y) < origin ? p.y + n.y * floatScale : p_i.y,
            pstd::abs(p.z) < origin ? p.z + n.z * floatScale : p_i.z};
}

inline Ray RayBetween(vec3 p0, vec3 p1) {
    Ray ray;
    ray.o = p0;
    ray.d = Normalize(p1 - p0, ray.tmax);
    ray.tmax = ray.tmax * (1.0f - 1e-3f);
    ray.tmin = ray.tmax * 1e-4f;
    return ray;
}

inline pstd::pair<float, float> TAfterTransform(const mat3& transform, const Ray& ray) {
    vec3 p0 = transform * ray.o;
    return {Distance(p0, transform * ray(ray.tmin)),
            ray.tmax > FloatMax ? FloatMax : Distance(p0, transform * ray(ray.tmax))};
}

inline Ray TransformRayOD(mat4 transform, Ray ray) {
    ray.o = transform * vec4(ray.o, 1.0f);
    ray.d = Normalize((mat3)transform * ray.d);
    return ray;
}

}  // namespace pine

#endif  // PINE_CORE_RAY_H