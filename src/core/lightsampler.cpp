#include <core/lightsampler.h>
#include <util/parameters.h>

namespace pine {

SampledLight UniformLightSampler::SampleLight(vec3, vec3, float ul) const {
    return SampleLight(ul);
}
SampledLight UniformLightSampler::SampleLight(float ul) const {
    if (lights.size() == 0)
        return {};
    uint32_t index = min(size_t(ul * lights.size()), lights.size() - 1);

    SampledLight sl;
    sl.light = &lights[index];
    sl.pdf = 1.0f / (int)lights.size();
    return sl;
}

PowerLightSampler::PowerLightSampler(const std::vector<Light>& lights) : lights(lights) {
    std::vector<float> lightPower;
    for (auto&& light : lights)
        lightPower.push_back(light.Power().y());
    powerDistr = Distribution1D(&lightPower[0], (int)lightPower.size());
}

SampledLight PowerLightSampler::SampleLight(vec3, vec3, float ul) const {
    return SampleLight(ul);
}
SampledLight PowerLightSampler::SampleLight(float ul) const {
    SampledLight sl;
    sl.light = &lights[powerDistr.SampleDiscrete(ul, sl.pdf)];
    return sl;
}

UniformLightSampler UniformLightSampler::Create(const Parameters&,
                                                const std::vector<Light>& lights) {
    return UniformLightSampler(lights);
}
PowerLightSampler PowerLightSampler::Create(const Parameters&, const std::vector<Light>& lights) {
    return PowerLightSampler(lights);
}

LightSampler LightSampler::Create(const Parameters& params, const std::vector<Light>& lights) {
    std::string type = params.GetString("type", "Power");
    SWITCH(type) {
        CASE("Uniform") return UniformLightSampler(UniformLightSampler::Create(params, lights));
        CASE("Power") return PowerLightSampler(PowerLightSampler::Create(params, lights));
        DEFAULT {
            LOG_WARNING("[LightSampler][Create]Unknown type \"&\"", type);
            return UniformLightSampler(UniformLightSampler::Create(params, lights));
        }
    }
}

}  // namespace pine