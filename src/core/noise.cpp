#include <core/noise.h>
#include <util/log.h>
#include <util/rng.h>

namespace pine {

float PerlinNoise(vec3 np, float frequency, int seed) {
    np *= frequency;
    vec3 uvw = np - Floor(np);
    uvw = uvw * uvw * (vec3(3) - 2 * uvw);

    vec3 noise[2][2][2];
    for (int x = 0; x < 2; x++)
        for (int y = 0; y < 2; y++)
            for (int z = 0; z < 2; z++) {
                vec2 u = RNG(Hash(vec3i((Floor(np) + vec3i(x, y, z))), seed)).Uniform2f();
                noise[x][y][z] = SphericalToCartesian(u.x * Pi * 2, u.y * Pi);
            }
    return 0.5f * (1.0f + PerlinInterp(noise, uvw));
}

vec3 PerlinNoise3D(vec3 np, float frequency, int seed) {
    return {PerlinNoise(np, frequency, seed), PerlinNoise(np, frequency, seed + 1),
            PerlinNoise(np, frequency, seed + 2)};
}

float Turbulence(vec3 np, float frequency, int octaves) {
    float accum = 0.0f;
    float weight = 1.0f;

    for (int i = 0; i < octaves; i++) {
        accum += weight * PerlinNoise(np, frequency);
        weight *= 0.5f;
        np *= 2.0f;
    }

    return pstd::sqr(accum / (2.0f - weight * 2));
}

vec3 Turbulence3D(vec3 np, float frequency, int octaves) {
    vec3 accum;
    float weight = 1.0f;

    for (int i = 0; i < octaves; i++) {
        accum += weight * PerlinNoise3D(np, frequency);
        weight *= 0.5f;
        np *= 2.0f;
    }

    return pstd::sqr(accum / (2.0f - weight * 2));
}

pstd::vector<vec2> GenerateWhiteNoise(vec2i size, int seed) {
    pstd::vector<vec2> noise(Area(size));
    RNG sampler(seed);
    for (int y = 0; y < size.y; y++)
        for (int x = 0; x < size.x; x++)
            noise[x + y * size.x] = sampler.Uniform2f();

    return noise;
}
pstd::vector<vec2> GenerateBlueNoise(vec2i size, int seed) {
    pstd::vector<vec2> noise(Area(size));
    RNG sampler(seed);
    for (int y = 0; y < size.y; y++)
        for (int x = 0; x < size.x; x++)
            noise[x + y * size.x] = sampler.Uniform2f();

    const int r = 1;
    const float sigma_i2 = 2.1f * 2.1f;
    const float sigma_s2 = 1.0f * 1.0f;
    // const int d = 2;
    const int numIterations = Area(size) * 2;

    auto Energy = [&](vec2i pi) {
        float sum = 0.0f;

        vec2 ps = noise[pi.x + pi.y * size.x];
        for (int y = pstd::max(0, pi.y - r); y <= pstd::min(size.y - 1, pi.y + r); y++)
            for (int x = pstd::max(0, pi.x - r); x <= pstd::min(size.x - 1, pi.x + r); x++) {
                vec2i qi = vec2i(x, y);
                if (pi == qi)
                    continue;
                vec2 qs = noise[qi.x + qi.y * size.x];
                sum += pstd::exp(-LengthSquared(pi - qi) / sigma_i2 - Length(ps - qs) / sigma_s2);
            }
        return sum;
    };

    for (int i = 0; i < numIterations; i++) {
        vec2i qi0 = sampler.Uniform2f() * size;
        vec2i qi1 = sampler.Uniform2f() * size;
        float E0 = Energy(qi0) + Energy(qi1);
        pstd::swap(noise[qi0.x + qi0.y * size.x], noise[qi1.x + qi1.y * size.x]);
        float E1 = Energy(qi0) + Energy(qi1);
        if (E1 > E0)
            pstd::swap(noise[qi0.x + qi0.y * size.x], noise[qi1.x + qi1.y * size.x]);
    }

    return noise;
}

}  // namespace pine