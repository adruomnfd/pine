#include <core/material.h>
#include <core/scattering.h>
#include <core/scene.h>
#include <util/parameters.h>
#include <util/log.h>

#include <algorithm>

namespace pine {

pstd::optional<BSDFSample> LayeredMaterial::Sample(const MaterialEvalCtx& c) const {
    pstd::optional<BSDFSample> bs;
    for (auto&& bsdf : bsdfs) {
        bs = bsdf.Sample(c.wi, c.u1, c.u2, c);
        if (bs && SameHemisphere(c.wi, bs->wo))
            break;
    }

    return bs;
}

Spectrum LayeredMaterial::F(const MaterialEvalCtx& c) const {
    vec3 f;
    for (auto&& bsdf : bsdfs)
        f += bsdf.F(c.wi, c.wo, c);
    return f;
}
float LayeredMaterial::PDF(const MaterialEvalCtx& c) const {
    float pdf = 0.0f;
    for (auto&& bsdf : bsdfs)
        pdf += bsdf.PDF(c.wi, c.wo, c);
    return pdf;
}

LayeredMaterial::LayeredMaterial(const Parameters& params) {
    pstd::vector<pstd::pair<int, Parameters>> layers;
    for (auto& layer : params)
        if (trim(layer.first, 0, 5) == "layer")
            layers.push_back({pstd::stoi(trim(layer.first, 5)), layer.second.back()});

    pstd::sort(layers, [](const auto& lhs, const auto& rhs) { return lhs.first > rhs.first; });

    for (auto& layer : layers)
        bsdfs.push_back(CreateBSDF(layer.second));
}

EmissiveMaterial::EmissiveMaterial(const Parameters& params) {
    color = CreateNode(params["color"]);
}

vec3 Material::BumpNormal(const MaterialEvalCtx& c) const {
    if (!bumpMap)
        return c.n;
    NodeEvalCtx c0 = c, c1 = c;
    const float delta = 0.01f;
    c0.uv += vec2(delta, 0.0f);
    c1.uv += vec2(0.0f, delta);
    c0.p += c.dpdu * delta;
    c1.p += c.dpdv * delta;
    float dddu = (bumpMap->EvalFloat(c0) - bumpMap->EvalFloat(c)) / delta;
    float dddv = (bumpMap->EvalFloat(c1) - bumpMap->EvalFloat(c)) / delta;
    vec3 dpdu = c.dpdu + dddu * c.n;
    vec3 dpdv = c.dpdv + dddv * c.n;
    return FaceSameHemisphere(Normalize(Cross(dpdu, dpdv)), c.n);
}

Material CreateMaterial(const Parameters& params) {
    pstd::string type = params.GetString("type");
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
        material.bumpMap = CreateNode(params["bumpMap"]);

    return material;
}

}  // namespace pine
