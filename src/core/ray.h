#ifndef PINE_CORE_RAY_H
#define PINE_CORE_RAY_H

#include <core/vecmath.h>
#include <util/string.h>

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

    Fstring Formatting(Format fmt) const {
        return Fstring(fmt, "[Ray]origin & direction & tmin & tmax &", o, d, tmin, tmax);
    }

    vec3 o;
    vec3 d;
    float tmin = 0.0f;
    float tmax = FloatMax;
    const Medium* medium = nullptr;
};

}  // namespace pine

#endif  // PINE_CORE_RAY_H