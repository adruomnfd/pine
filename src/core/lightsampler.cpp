#include <core/lightsampler.h>
#include <util/parameters.h>

namespace pine {

SampledLight UniformLightSampler::SampleLight(vec3, vec3, float ul) const {
    if (lights.size() == 0)
        return {};
    uint32_t index = min(size_t(ul * lights.size()), lights.size() - 1);

    SampledLight sl;
    sl.light = &lights[index];
    sl.pdf = Pdf();
    return sl;
}
SampledLight UniformLightSampler::SampleLight(float ul) const {
    if (lights.size() == 0)
        return {};
    uint32_t index = min(size_t(ul * lights.size()), lights.size() - 1);

    SampledLight sl;
    sl.light = &lights[index];
    sl.pdf = Pdf();
    return sl;
}

UniformLightSampler UniformLightSampler::Create(const Parameters&,
                                                const std::vector<Light>& lights) {
    return UniformLightSampler(lights);
}

LightSampler LightSampler::Create(const Parameters& params, const std::vector<Light>& lights) {
    std::string type = params.GetString("type", "Uniform");
    SWITCH(type) {
        CASE("Uniform") return UniformLightSampler(UniformLightSampler::Create(params, lights));
        DEFAULT {
            LOG_WARNING("[LightSampler][Create]Unknown type \"&\"", type);
            return UniformLightSampler(UniformLightSampler::Create(params, lights));
        }
    }
}

}  // namespace pine