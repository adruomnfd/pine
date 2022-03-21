#include <core/light.h>
#include <core/color.h>
#include <core/geometry.h>
#include <core/sampling.h>
#include <util/parameters.h>

namespace pine {

LightSample PointLight::Sample(vec3 p, vec2) const {
    LightSample ls;
    ls.wo = Normalize(position - p, ls.distance);
    ls.pdf = 1.0f;
    ls.Le = color;
    return ls;
}

LightSample DirectionalLight::Sample(vec3, vec2) const {
    LightSample ls;
    ls.Le = color;
    ls.wo = direction;
    ls.distance = 1e+10f;
    ls.pdf = 1.0f;
    return ls;
}

LightSample AreaLight::Sample(vec3 p, vec2 u) const {
    return shape->Sample(p, u);
}

LightSample Atmosphere::Sample(vec3, vec2) const {
    LightSample ls;
    ls.distance = 1e+10f;
    ls.wo = sunDirection;
    ls.pdf = 1.0f;
    ls.Le = AtmosphereColor(ls.wo, sunDirection, sunColor);
    return ls;
}

Spectrum Atmosphere::Color(vec3 wo) const {
    return AtmosphereColor(wo, sunDirection, sunColor);
}

PointLight PointLight::Create(const Parameters& params) {
    return PointLight(params.GetVec3("position"), params.GetVec3("color"));
}

DirectionalLight DirectionalLight::Create(const Parameters& params) {
    return DirectionalLight(params.GetVec3("direction"), params.GetVec3("color"));
}

Atmosphere Atmosphere::Create(const Parameters& params) {
    return Atmosphere(params.GetVec3("sunDirection", vec3(2, 6, 3)),
                      params.GetVec3("sunColor", vec3(1.0f)));
}

EnvironmentLight EnvironmentLight::Create(const Parameters& lightParams) {
    Parameters params = lightParams["environment"];
    std::string type = params.GetString("type");
    SWITCH(type) {
        CASE("Atmosphere") return Atmosphere(Atmosphere::Create(params));
        DEFAULT {
            LOG_WARNING("[EnvironmentLight][Create]Unknown type \"&\"", type);
            return Atmosphere(Atmosphere::Create(params));
        }
    }
}

Light Light::Create(const Parameters& params) {
    std::string type = params.GetString("type");
    SWITCH(type) {
        CASE("Point") return PointLight(PointLight::Create(params));
        CASE("Directional") return DirectionalLight(DirectionalLight::Create(params));
        CASE("Environment") return EnvironmentLight(EnvironmentLight::Create(params));
        DEFAULT {
            LOG_WARNING("[Light][Create]Unknown type \"&\"", type);
            return PointLight(PointLight::Create(params));
        }
    }
}

}  // namespace pine