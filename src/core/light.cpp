#include <core/light.h>
#include <core/scene.h>
#include <core/sampling.h>
#include <util/parameters.h>

namespace pine {

LightSample PointLight::Sample(vec3 p, float, vec2) const {
    LightSample ls;
    ls.p = position;
    ls.wo = Normalize(ls.p - p, ls.distance);
    ls.pdf = Sqr(ls.distance);
    ls.Le = color;
    ls.isDelta = true;
    return ls;
}
LightEmissionSample PointLight::SampleEmission(float, vec2, vec2 ud) const {
    LightEmissionSample les;
    les.Le = color;
    les.p = position;
    les.wo = UniformSphereSampling(ud);
    return les;
}

LightSample DirectionalLight::Sample(vec3, float, vec2) const {
    LightSample ls;
    ls.Le = color;
    ls.wo = direction;
    ls.distance = 1e+10f;
    ls.p = ls.wo * ls.distance;
    ls.pdf = 1.0f;
    ls.isDelta = true;
    return ls;
}
LightEmissionSample DirectionalLight::SampleEmission(float, vec2, vec2) const {
    return {};
}

LightSample AreaLight::Sample(vec3 p, float, vec2 u2) const {
    LightSample ls;
    ls.p = position + (u2[0] - 0.5f) * ex + (u2[1] - 0.5f) * ey;
    ls.wo = Normalize(ls.p - p, ls.distance);
    ls.pdf = Sqr(ls.distance) / AbsDot(ls.wo, n) / area;
    ls.Le = color;
    ls.isDelta = false;
    return ls;
}
LightEmissionSample AreaLight::SampleEmission(float, vec2 up, vec2 ud) const {
    LightEmissionSample les;
    les.Le = color;
    les.p = position + (up[0] - 0.5f) * ex + (up[1] - 0.5f) * ey;
    les.wo = CoordinateSystem(n) * CosineWeightedSampling(ud);
    return les;
}

LightSample MeshLight::Sample(vec3, float, vec2) const {
    return {};
}
LightEmissionSample MeshLight::SampleEmission(float, vec2, vec2) const {
    return {};
}

PointLight PointLight::Create(const Parameters& params) {
    return PointLight(params.GetVec3("position"), params.GetVec3("color"));
}

DirectionalLight DirectionalLight::Create(const Parameters& params) {
    if (params.Has("transformX")) {
        mat4 transform = {params.GetVec4("transformX"), params.GetVec4("transformY"),
                          params.GetVec4("transformZ"), params.GetVec4("transformW")};
        return DirectionalLight(transform * vec4(0, 0, 1, 0), params.GetVec3("color"));
    }
    return DirectionalLight(params.GetVec3("direction"), params.GetVec3("color"));
}

AreaLight AreaLight::Create(const Parameters& params) {
    if (params.Has("transformX")) {
        mat4 transform = {params.GetVec4("transformX"), params.GetVec4("transformY"),
                          params.GetVec4("transformZ"), params.GetVec4("transformW")};
        LOG("&", transform);
        return AreaLight(transform.w, (mat3)transform * vec3(params.GetFloat("sizex"), 0.0f, 0.0f),
                         (mat3)transform * vec3(0.0f, params.GetFloat("sizey"), 0.0f),
                         params.GetVec3("color"));
    }
    return AreaLight(params.GetVec3("position"), params.GetVec3("ex"), params.GetVec3("ey"),
                     params.GetVec3("color"));
}

MeshLight MeshLight::Create(const Parameters&, const Scene*) {
    LOG_FATAL("[MeshLight][Create]Unimplmented");
    return MeshLight(nullptr);
}

Light Light::Create(const Parameters& params, const Scene* scene) {
    std::string type = params.GetString("type");
    if (type == "Point") {
        return new PointLight(PointLight::Create(params));
    } else if (type == "Directional") {
        return new DirectionalLight(DirectionalLight::Create(params));
    } else if (type == "Area") {
        return new AreaLight(AreaLight::Create(params));
    } else if (type == "Mesh") {
        return new MeshLight(MeshLight::Create(params, scene));
    } else {
        return new PointLight(PointLight::Create(params));
    }
}

}  // namespace pine