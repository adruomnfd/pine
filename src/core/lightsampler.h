#ifndef PINE_CORE_LIGHTSAMPLER
#define PINE_CORE_LIGHTSAMPLER

#include <core/light.h>
#include <util/taggedvariant.h>

#include <vector>

namespace pine {

struct UniformLightSampler {
    static UniformLightSampler Create(const Parameters& params, const std::vector<Light>& lights);
    UniformLightSampler(const std::vector<Light>& lights) : lights(lights) {
    }

    LightSample Sample(vec3 p, float ul, vec2 ud) const;
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

    LightSample Sample(vec3 p, float ul, vec2 ud) const {
        LightSample ls = Dispatch([&](auto&& x) { return x.Sample(p, ul, ud); });
        ls.pdf *= Pdf();
        return ls;
    }
    float Pdf() const {
        return Dispatch([&](auto&& x) { return x.Pdf(); });
    }
};

}  // namespace pine

#endif  // PINE_CORE_LIGHTSAMPLER