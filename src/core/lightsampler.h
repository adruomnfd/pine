#ifndef PINE_CORE_LIGHTSAMPLER
#define PINE_CORE_LIGHTSAMPLER

#include <core/light.h>
#include <util/taggedvariant.h>

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
    float Pdf() const {
        if (!lights.size())
            return 0.0f;
        return 1.0f / (float)lights.size();
    }

    std::vector<Light> lights;
};

struct LightSampler : TaggedVariant<UniformLightSampler> {
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