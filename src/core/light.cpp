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
    ls.isDelta = true;
    return ls;
}

LightSample DirectionalLight::Sample(vec3, vec2) const {
    LightSample ls;
    ls.Le = color;
    ls.wo = direction;
    ls.distance = 1e+10f;
    ls.pdf = 1.0f;
    ls.isDelta = true;
    return ls;
}

LightSample AreaLight::Sample(vec3 p, vec2 u) const {
    return shape->Sample(p, u);
}

Atmosphere::Atmosphere(vec3 sundir, vec3 suncol, vec2i size, bool interpolate)
    : sunDirection(Normalize(sundir)), sunColor(suncol), size(size), interpolate(interpolate) {
    colors.resize(Area(size));
    for (int y = 0; y < size.y; y++) {
        for (int x = 0; x < size.x; x++) {
            vec3 d = UniformSphereMampling(vec2((float)x / size.x, (float)y / size.y));
            vec3 color = AtmosphereColor(d, sunDirection, sunColor).ToRGB();
            colors[y * size.x + x] = color.HasNaN() ? vec3(0.0f) : color;
        }
    }
    sunSampledColor = AtmosphereColor(sunDirection, sunDirection, sunColor);
};

LightSample Atmosphere::Sample(vec3, vec2 u) const {
    LightSample ls;
    ls.distance = 1e+10f;
    if (u.x < 0.5f) {
        u.x *= 2.0f;
        ls.wo = UniformSphereMampling(u);
        ls.pdf = 1.0f / Pi4;
        vec2i st = Min(u * size, size - vec2(1));
        ls.Le = colors[st.y * size.x + st.x];
    } else {
        ls.wo = sunDirection;
        ls.pdf = 1.0f;
        ls.Le = sunSampledColor.ToRGB();
        ls.isDelta = true;
    }
    ls.pdf /= 2;

    return ls;
}

Spectrum Atmosphere::Color(vec3 wo) const {
    vec2 uv = InverseUniformSphereMampling(wo) * size;
    if (!interpolate) {
        vec2i st = Min(uv, size - vec2(1));
        return colors[st.y * size.x + st.x];
    }
    vec2i st00 = {std::floor(uv.x), std::floor(uv.y)};
    vec2i st01 = {std::ceil(uv.x), std::floor(uv.y)};
    vec2i st10 = {std::floor(uv.x), std::ceil(uv.y)};
    vec2i st11 = {std::ceil(uv.x), std::ceil(uv.y)};
    st00 %= size;
    st01 %= size;
    st10 %= size;
    st11 %= size;
    vec2 p = uv - st00;
    vec3 c00 = colors[st00.y * size.x + st00.x];
    vec3 c01 = colors[st01.y * size.x + st01.x];
    vec3 c10 = colors[st10.y * size.x + st10.x];
    vec3 c11 = colors[st11.y * size.x + st11.x];
    vec3 c0 = Lerp(p.x, c00, c01);
    vec3 c1 = Lerp(p.x, c10, c11);
    return Lerp(p.y, c0, c1);
}
float Atmosphere::Pdf(vec3) const {
    return 1.0f / Pi4;
}

PointLight PointLight::Create(const Parameters& params) {
    return PointLight(params.GetVec3("position"), params.GetVec3("color"));
}

DirectionalLight DirectionalLight::Create(const Parameters& params) {
    return DirectionalLight(params.GetVec3("direction"), params.GetVec3("color"));
}

Atmosphere Atmosphere::Create(const Parameters& params) {
    return Atmosphere(params.GetVec3("sunDirection", vec3(2, 6, 3)),
                      params.GetVec3("sunColor", vec3(1.0f)), params.GetVec2i("size", vec2i(512)),
                      params.GetBool("interpolate", true));
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