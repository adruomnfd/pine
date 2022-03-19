#include <core/lightsampler.h>
#include <util/parameters.h>

namespace pine {

LightSample UniformLightSampler::Sample(vec3 p, float ul, vec2 ud) {
    if (lights.size() == 0)
        return {};
    uint32_t index = min(size_t(ul * lights.size()), lights.size() - 1);
    return lights[index].Sample(p, ud);
}

UniformLightSampler UniformLightSampler::Create(const Parameters&,
                                                const std::vector<Light>& lights) {
    return UniformLightSampler(lights);
}

LightSampler LightSampler::Create(const Parameters& params, const std::vector<Light>& lights) {
    std::string type = params.GetString("type");
    SWITCH(type) {
        CASE("Uniform") return UniformLightSampler(UniformLightSampler::Create(params, lights));
        DEFAULT {
            LOG_WARNING("[LightSampler][Create]Unknown type \"&\"", type);
            return UniformLightSampler(UniformLightSampler::Create(params, lights));
        }
    }
}

}  // namespace pine