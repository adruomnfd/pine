#include <core/material.h>
#include <core/scattering.h>
#include <core/scene.h>
#include <util/log.h>
#include <util/profiler.h>
#include <util/parameters.h>

#include <algorithm>

namespace pine {

BSDFSample LayeredMaterial::Sample(MaterialEvalContext c) const {
    BSDFSample bs;
    for (auto bsdf : bsdfs) {
        bs = bsdf.Sample(c.wi, c.u1, c.u2, c);
        if (SameHemisphere(c.wi, bs.wo))
            break;
    }

    return bs;
}

vec3 LayeredMaterial::F(MaterialEvalContext c) const {
    vec3 f;
    for (auto bsdf : bsdfs)
        f += bsdf.F(c.wi, c.wo, c);
    return f;
}
float LayeredMaterial::PDF(MaterialEvalContext c) const {
    float pdf = 0.0f;
    for (auto bsdf : bsdfs)
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

BSDFSample Material::Sample(MaterialEvalContext c) const {
    SampledProfiler _(ProfilePhase::MaterialSample);
    if (Ptr() == nullptr)
        return {};
    return Dispatch([&](auto ptr) {
        mat3 m2w = CoordinateSystem(c.n);
        mat3 w2m = Inverse(m2w);
        c.wi = w2m * c.wi;
        c.wo = w2m * c.wo;
        c.n = vec3(0, 0, 1);

        BSDFSample bs = ptr->Sample(c);

        bs.wo = m2w * bs.wo;
        return bs;
    });
}

vec3 Material::F(MaterialEvalContext c) const {
    SampledProfiler _(ProfilePhase::MaterialSample);
    if (Ptr() == nullptr)
        return {};
    return Dispatch([&](auto ptr) {
        mat3 w2m = Inverse(CoordinateSystem(c.n));
        c.wi = w2m * c.wi;
        c.wo = w2m * c.wo;
        c.n = vec3(0, 0, 1);
        return ptr->F(c);
    });
}
float Material::PDF(MaterialEvalContext c) const {
    SampledProfiler _(ProfilePhase::MaterialSample);
    if (Ptr() == nullptr)
        return {};
    return Dispatch([&](auto ptr) {
        mat3 w2m = Inverse(CoordinateSystem(c.n));
        c.wi = w2m * c.wi;
        c.wo = w2m * c.wo;
        c.n = vec3(0, 0, 1);
        return ptr->PDF(c);
    });
}

vec3 Material::Le(MaterialEvalContext c) const {
    SampledProfiler _(ProfilePhase::MaterialSample);
    if (Ptr() == nullptr)
        return {};
    return Dispatch([&](auto ptr) {
        mat3 w2m = Inverse(CoordinateSystem(c.n));
        c.wi = w2m * c.wi;
        c.wo = w2m * c.wo;
        c.n = vec3(0, 0, 1);
        return ptr->Le(c);
    });
}

Material Material::Create(const Parameters& params) {
    std::string type = params.GetString("type");
    SWITCH(type) {
        CASE("Layered") return new LayeredMaterial(params);
        CASE("Emissive") return new EmissiveMaterial(params);
        DEFAULT {
            LOG_WARNING("[Material][Create]Unknown type \"&\"", type);
            return new LayeredMaterial(params);
        }
    }
}

}  // namespace pine
