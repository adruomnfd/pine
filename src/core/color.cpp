#include <core/color.h>
#include <core/geometry.h>

namespace pine {

vec3 Uncharted2Flimic(vec3 v) {
    vec3 A = (vec3)0.15f, B = (vec3)0.50f, C = (vec3)0.10f, D = (vec3)0.20f, E = (vec3)0.02f,
         F = (vec3)0.30f;
    auto mapping = [=](vec3 x) {
        return (x * (A * x + C * B) + D * E) / (x * (A * x + B) + D * F) - E / F;
    };
    return mapping(v * 2.0f) * vec3(1.0f) / mapping(vec3(11.2f));
}

vec3 ACES(vec3 v) {
    float a = 2.51f;
    vec3 b = vec3(0.03f);
    float c = 2.43f;
    vec3 d = vec3(0.59f);
    vec3 e = vec3(0.14f);
    v = v * (a * v + b) / (v * (c * v + d) + e);
    return Clamp(Pow(0.8f * v * (a * v + b) / (v * (c * v + d) + e), 0.8f), vec3(0.0f), vec3(1.0f));
}

vec3 ColorMap(float v) {
    if (v < 1 / 3.0f) {
        return Lerp(v * 3.0f, vec3(0), vec3(0.0f, 0.0f, 0.5f));
    } else if (v < 2 / 3.0f) {
        return Lerp((v - 1 / 3.0f) * 3.0f, vec3(0.0f, 0.0f, 0.5f), vec3(0, 1, 0));
    } else if (v < 3 / 3.0f) {
        return Lerp((v - 2 / 3.0f) * 3.0f, vec3(0, 1, 0), vec3(1, 0, 0));
    } else {
        return vec3(1, 0, 0);
    }
}

Spectrum AtmosphereColor(vec3 direction, vec3 sunDirection, vec3 sunColor) {
    const vec3 betaR = vec3(3.8e-6f, 13.5e-6f, 33.1e-6f), betaM = vec3(21e-6f);
    const float atmosphereRadius = 6420e3f, earthRadius = 6360e3f;
    const float Hr = 1.0f / 7995.0f,
                Hm = (direction == sunDirection) ? 1.0f / 10000.0f : 1.0f / 1200.0f;
    const int nSamples = 8, nSamplesLight = 4;
    const float mu = Dot(direction, sunDirection);
    const float phaseR = 3.0f / (16.0f * Pi) * (1.0f + mu * mu), g = 0.76f,
                phaseM = 3.0f / (8.0f * Pi) * (1.0f - g * g) * (1.0f + mu * mu) /
                         ((2.0f + g * g) * powf(1.0f + g * g - 2.0f * g * mu, 1.5f));

    Ray ray = Ray(vec3(0.0f, earthRadius, 0.0f), direction);
    ray.tmax = Sphere::ComputeT(ray.o, ray.d, 0.0f, vec3(0.0f), atmosphereRadius);
    float segmentLength = ray.tmax / nSamples, tCurrent = 0.0f;
    float opticalDepthR = 0.0f, opticalDepthM = 0.0f;
    vec3 sumR, sumM;

    for (int i = 0; i < nSamples; i++) {
        vec3 samplePosition = ray(tCurrent + segmentLength * 0.5f);
        float height = Length(samplePosition) - earthRadius;
        float hr = std::exp(-height * Hr) * segmentLength,
              hm = std::exp(-height * Hm) * segmentLength;
        opticalDepthR += hr;
        opticalDepthM += hm;

        float ltmax =
            Sphere::ComputeT(samplePosition, sunDirection, 0.0f, vec3(0.0f), atmosphereRadius);
        float segmentLengthLight = ltmax / nSamplesLight;
        float opticalDepthRLight = 0.0f, opticalDepthMLight = 0.0f;

        int j = 0;
        float lo2 = Dot(samplePosition, samplePosition);
        float ld2 = Dot(sunDirection, sunDirection);
        float lod = Dot(samplePosition, sunDirection);
        float lt = segmentLengthLight * 0.5f;
        for (; j < nSamplesLight; j++, lt += segmentLengthLight) {
            float heightLight = lo2 + Sqr(lt) * ld2 + 2 * lt * lod - earthRadius;
            if (heightLight < 0)
                break;
            opticalDepthRLight += std::exp(-heightLight * Hr) * segmentLengthLight;
            opticalDepthMLight += std::exp(-heightLight * Hm) * segmentLengthLight;
        }
        if (j == nSamplesLight) {
            vec3 tau = betaR * (opticalDepthR + opticalDepthRLight) +
                       betaM * (opticalDepthM + opticalDepthMLight);
            vec3 tr = Exp(-tau);
            sumR += tr * hr;
            sumM += tr * hm;
        }
        tCurrent += segmentLength;
    }
    return (sumR * betaR * phaseR + sumM * betaM * phaseM) * 10.0f * sunColor;
}

Spectrum SkyColor(vec3 direction, vec3 sunDirection, vec3 sunColor) {
    if (sunDirection == direction)
        return vec3(8.0f * sunColor);
    std::swap(direction.y, direction.z);
    auto [phi, theta] = CartesianToSpherical(direction);
    theta /= Pi;
    return sunColor * Sqr(Lerp(theta, vec3(0.4f, 0.6f, 0.8f), vec3(0.01f, 0.03f, 0.3f)));
}

}  // namespace pine