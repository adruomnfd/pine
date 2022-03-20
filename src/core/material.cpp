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

vec3 LayeredMaterial::F(const MaterialEvalContext& c) const {
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
    for (auto& layer : params.subset) {
        if (layer.first.substr(0, 5) == "layer")
            layers.push_back({std::stoi(layer.first.substr(5)), layer.second});
    }

    std::sort(layers.begin(), layers.end(),
              [](const auto& lhs, const auto& rhs) { return lhs.first > rhs.first; });

    for (auto& layer : layers) {
        bsdfs.push_back(BSDF::Create(layer.second));
    }
}

EmissiveMaterial::EmissiveMaterial(const Parameters& params) {
    color = Node::Create(params["color"]);
}

std::optional<BSDFSample> Material::Sample(const MaterialEvalContext& c) const {
    SampledProfiler _(ProfilePhase::MaterialSample);
    return Dispatch([&](auto&& x) {
        std::optional<BSDFSample> bs = x.Sample(c);
        if (bs)
            bs->wo = c.n2w * bs->wo;
        return bs;
    });
}

Material Material::Create(const Parameters& params) {
    std::string type = params.GetString("type");
    SWITCH(type) {
        CASE("Layered") return LayeredMaterial(params);
        CASE("Emissive") return EmissiveMaterial(params);
        DEFAULT {
            LOG_WARNING("[Material][Create]Unknown type \"&\"", type);
            return LayeredMaterial(params);
        }
    }
}

}  // namespace pine
