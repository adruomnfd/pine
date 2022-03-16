#ifndef PINE_CORE_NOISE_H
#define PINE_CORE_NOISE_H

#include <core/vecmath.h>

#include <vector>

namespace pine {

float PerlinNoise(vec3 np, float frequency = 1.0f, int seed = 0);

vec3 PerlinNoise3D(vec3 np, float frequency = 1.0f, int seed = 0);

float Turbulence(vec3 np, float frequency, int octaves);

vec3 Turbulence3D(vec3 np, float frequency, int octaves);

std::vector<vec2> GenerateWhiteNoise(vec2i size, int seed);
std::vector<vec2> GenerateBlueNoise(vec2i size, int seed);

}  // namespace pine

#endif  // PINE_CORE_NOISE_H