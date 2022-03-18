#ifndef PINE_CORE_LIGHTSAMPLER
#define PINE_CORE_LIGHTSAMPLER

#include <core/light.h>
#include <util/taggedptr.h>

#include <vector>

namespace pine {

struct UniformLightSampler {
    static UniformLightSampler Create(const Parameters& params, const std::vector<Light>& lights);
    UniformLightSampler(const std::vector<Light>& lights) : lights(lights) {
    }

    LightSample Sample(vec3 p, float ul, vec2 ud);

    std::vector<Light> lights;
};

struct LightSampler : TaggedPointer<UniformLightSampler> {
    using TaggedPointer::TaggedPointer;
    static LightSampler Create(const Parameters& params, const std::vector<Light>& lights);

    LightSample Sample(vec3 p, float ul, vec2 ud) {
        return Dispatch([&](auto ptr) { return ptr->Sample(p, ul, ud); });
    }
};

}  // namespace pine

#endif  // PINE_CORE_LIGHTSAMPLER