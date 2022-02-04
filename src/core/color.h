#ifndef PINE_CORE_COLOR_H
#define PINE_CORE_COLOR_H

#include <core/vecmath.h>

namespace pine {

inline vec3 GammaCorrect(vec3 v) {
    return Pow(v, 1.0f / 2.2f);
}

inline float Luminance(vec3 color) {
    return color.x * 0.212671f + color.y * 0.715160f + color.z * 0.072169f;
}

vec3 Uncharted2Flimic(vec3 v);

vec3 ACES(vec3 v);

vec3 ColorMap(float v);

vec3 AtmosphereColor(vec3 direction, vec3 sunDirection, float sunIntensity);
vec3 SkyColor(vec3 direction, vec3 sunDirection, float sunIntensity);

}  // namespace pine

#endif  // PINE_CORE_COLOR_H