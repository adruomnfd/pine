#ifndef PINE_CORE_LIGHTSAMPLER
#define PINE_CORE_LIGHTSAMPLER

#include <core/light.h>
#include <util/taggedvariant.h>
#include <util/distribution.h>

#include <vector>

namespace pine {

struct SampledLight {
    const Light* light = nullptr;
    float pdf = 0.0f;
};

struct UniformLightSampler {
    static UniformLightSampler Create(const Parameters& params, const std::vector<Light>& lights);
    UniformLightSampler(const std::vector<Light>& lights) : lights(lights) {
    }

    SampledLight SampleLight(vec3 p, vec3 n, float ul) const;
    SampledLight SampleLight(float ul) const;

    std::vector<Light> lights;
};

struct PowerLightSampler {
    static PowerLightSampler Create(const Parameters& params, const std::vector<Light>& lights);
    PowerLightSampler(const std::vector<Light>& lights);

    SampledLight SampleLight(vec3 p, vec3 n, float ul) const;
    SampledLight SampleLight(float ul) const;

    std::vector<Light> lights;
    Distribution1D powerDistr;
};

struct LightSampler : TaggedVariant<UniformLightSampler, PowerLightSampler> {
    using TaggedVariant::TaggedVariant;
    static LightSampler Create(const Parameters& params, const std::vector<Light>& lights);

    SampledLight SampleLight(vec3 p, vec3 n, float ul) const {
        return Dispatch([&](auto&& x) { return x.SampleLight(p, n, ul); });
    }
    SampledLight SampleLight(float ul) const {
        return Dispatch([&](auto&& x) { return x.SampleLight(ul); });
    }
};

}  // namespace pine

#endif  // PINE_CORE_LIGHTSAMPLER