#ifndef PINE_CORE_LIGHTSAMPLER
#define PINE_CORE_LIGHTSAMPLER

#include <core/light.h>
#include <util/taggedvariant.h>
#include <util/distribution.h>

#include <pstd/vector.h>

namespace pine {

struct SampledLight {
    const Light* light = nullptr;
    float pdf = 0.0f;
};

struct UniformLightSampler {
    static UniformLightSampler Create(const Parameters& params, const pstd::vector<Light>& lights);
    UniformLightSampler(const pstd::vector<Light>& lights) : lights(lights) {
    }

    SampledLight SampleLight(vec3 p, vec3 n, float ul) const;
    SampledLight SampleLight(float ul) const;

    pstd::vector<Light> lights;
};

struct PowerLightSampler {
    static PowerLightSampler Create(const Parameters& params, const pstd::vector<Light>& lights);
    PowerLightSampler(const pstd::vector<Light>& lights);

    SampledLight SampleLight(vec3 p, vec3 n, float ul) const;
    SampledLight SampleLight(float ul) const;

    pstd::vector<Light> lights;
    Distribution1D powerDistr;
};

struct LightSampler : TaggedVariant<UniformLightSampler, PowerLightSampler> {
    using TaggedVariant::TaggedVariant;

    SampledLight SampleLight(vec3 p, vec3 n, float ul) const {
        return Dispatch([&](auto&& x) { return x.SampleLight(p, n, ul); });
    }
    SampledLight SampleLight(float ul) const {
        return Dispatch([&](auto&& x) { return x.SampleLight(ul); });
    }
};

LightSampler CreateLightSampler(const Parameters& params, const pstd::vector<Light>& lights);

}  // namespace pine

#endif  // PINE_CORE_LIGHTSAMPLER