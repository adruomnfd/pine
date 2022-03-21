#include <core/material.h>
#include <core/scattering.h>
#include <core/scene.h>
#include <util/parameters.h>
#include <util/log.h>

#include <algorithm>

namespace pine {

std::optional<BSDFSample> LayeredMaterial::Sample(const MaterialEvalContext& c) const {
    std::optional<BSDFSample> bs;
    for (auto&& bsdf : bsdfs) {
        bs = bsdf.Sample(c.wi, c.u1, c.u2, c);
        if (bs && SameHemisphere(c.wi, bs->wo))
            break;
    }

    return bs;
}

Spectrum LayeredMaterial::F(const MaterialEvalContext& c) const {
    vec3 f;
    for (auto&& bsdf : bsdfs)
        f += bsdf.F(c.wi, c.wo, c);
    return f;
}
float LayeredMaterial::PDF(const MaterialEvalContext& c) const {
    float pdf = 0.0f;
    for (auto&& bsdf : bsdfs)
        pdf += bsdf.PDF(c.wi, c.wo, c);
    return pdf;
}

LayeredMaterial::LayeredMaterial(const Parameters& params) {
    std::vector<std::pair<int, Parameters>> layers;
    for (auto& layer : params)
        if (layer.first.substr(0, 5) == "layer")
            layers.push_back({std::stoi(layer.first.substr(5)), layer.second.back()});

    std::sort(layers.begin(), layers.end(),
              [](const auto& lhs, const auto& rhs) { return lhs.first > rhs.first; });

    for (auto& layer : layers)
        bsdfs.push_back(BSDF::Create(layer.second));
}

EmissiveMaterial::EmissiveMaterial(const Parameters& params) {
    color = Node::Create(params["color"]);
}

vec3 Material::BumpNormal(const MaterialEvalContext& c) const {
    if (!bumpMap)
        return c.n;
    NodeEvalContext c0 = c, c1 = c;
    const float delta = 0.01f;
    c0.uv += vec2(delta, 0.0f);
    c1.uv += vec2(0.0f, delta);
    c0.p += c.dpdu * delta;
    c1.p += c.dpdv * delta;
    float dddu = (bumpMap->EvalFloat(c0) - bumpMap->EvalFloat(c)) / delta;
    float dddv = (bumpMap->EvalFloat(c1) - bumpMap->EvalFloat(c)) / delta;
    vec3 dpdu = c.dpdu + dddu * c.n;
    vec3 dpdv = c.dpdv + dddv * c.n;
    return Normalize(Cross(dpdu, dpdv));
}

Material Material::Create(const Parameters& params) {
    std::string type = params.GetString("type");
    Material material;

    SWITCH(type) {
        CASE("Layered") material = LayeredMaterial(params);
        CASE("Emissive") material = EmissiveMaterial(params);
        DEFAULT {
            LOG_WARNING("[Material][Create]Unknown type \"&\"", type);
            material = LayeredMaterial(params);
        }
    }

    if (params.HasSubset("bumpMap"))
        material.bumpMap = Node::Create(params["bumpMap"]);

    return material;
}

}  // namespace pine
