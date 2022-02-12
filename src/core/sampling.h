#ifndef PINE_CORE_SAMPLING_H
#define PINE_CORE_SAMPLING_H

#include <core/vecmath.h>

namespace pine {

inline vec2 SampleDiskPolar(vec2 u) {
    float r = std::sqrt(u[0]);
    float theta = 2 * Pi * u[1];
    return {r * std::cos(theta), r * std::sin(theta)};
}

inline vec2 SampleDiskConcentric(vec2 u) {
    u = vec2(u.x * 2 - 1.0f, u.y * 2 - 1.0f);
    float theta, r;
    if (fabsf(u.x) > fabsf(u.y)) {
        r = u.x;
        theta = Pi / 4.0f * u.y / u.x;
    } else {
        r = u.y;
        theta = Pi / 2.0f - Pi / 4.0f * (u.x / u.y);
    }
    return r * vec2(std::cos(theta), std::sin(theta));
}

inline vec3 CosineWeightedSampling(vec2 u) {
    vec2 d = SampleDiskConcentric(u);
    float z = sqrtf(fmaxf(1.0f - d.x * d.x - d.y * d.y, 0.0f));
    return vec3(d.x, d.y, z);
}

inline vec3 UniformSphereSampling(vec2 u) {
    return SphericalToCartesian(u.x * Pi * 2, u.y * Pi);
}

inline vec3 UniformHemisphereSampling(vec2 u) {
    return SphericalToCartesian(u.x * Pi * 2, u.y * Pi * 0.5f);
}

inline float BalanceHeuristic(int nF, float pF, int nG, float pG) {
    return nF * pF / (nF * pF + nG * pG);
}
inline float PowerHeuristic(int nF, float pF, int nG, float pG) {
    float f = nF * pF;
    float g = nG * pG;
    return Sqr(f) / (Sqr(f) + Sqr(g));
}

}  // namespace pine

#endif  // PINE_CORE_SAMPLING_H
