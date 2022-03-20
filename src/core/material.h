#ifndef PINE_CORE_MATERIAL_H
#define PINE_CORE_MATERIAL_H

#include <core/node.h>
#include <core/bxdf.h>
#include <util/taggedvariant.h>
#include <util/profiler.h>

#include <vector>

namespace pine {

struct MaterialEvalContext : NodeEvalContext {
    MaterialEvalContext(vec3 p, vec3 n, vec2 uv, vec3 wi, vec3 wo = vec3(0.0f),
                        vec2 u2 = vec2(0.0f), float u1 = 0.0f)
        : NodeEvalContext(p, n, uv), u2(u2), u1(u1), n2w(CoordinateSystem(n)) {
        mat3 w2n = Inverse(n2w);
        this->wi = w2n * wi;
        if (wo != vec3(0.0f))
            this->wo = w2n * wo;
    };

    vec3 wi;
    vec3 wo;
    vec2 u2;
    float u1;
    mat3 n2w;
};

struct LayeredMaterial {
    LayeredMaterial(const Parameters& params);

    std::optional<BSDFSample> Sample(const MaterialEvalContext& c) const;
    vec3 F(const MaterialEvalContext& c) const;
    float PDF(const MaterialEvalContext& c) const;
    vec3 Le(const MaterialEvalContext&) const {
        return {};
    }

    std::vector<BSDF> bsdfs;
};

struct EmissiveMaterial {
    EmissiveMaterial(const Parameters& params);

    std::optional<BSDFSample> Sample(const MaterialEvalContext&) const {
        return std::nullopt;
    }
    vec3 F(const MaterialEvalContext&) const {
        return {};
    }
    float PDF(const MaterialEvalContext&) const {
        return {};
    }
    vec3 Le(const MaterialEvalContext& c) const {
        return color.EvalVec3(c);
    }

    NodeInput color;
};

struct Material : public TaggedVariant<LayeredMaterial, EmissiveMaterial> {
  public:
    using TaggedVariant::TaggedVariant;
    static Material Create(const Parameters& params);

    std::optional<BSDFSample> Sample(const MaterialEvalContext& c) const;
    vec3 F(const MaterialEvalContext& c) const {
        SampledProfiler _(ProfilePhase::MaterialSample);
        return Dispatch([&](auto&& x) { return x.F(c); });
    }
    float PDF(const MaterialEvalContext& c) const {
        SampledProfiler _(ProfilePhase::MaterialSample);
        return Dispatch([&](auto&& x) { return x.PDF(c); });
    }
    vec3 Le(const MaterialEvalContext& c) const {
        SampledProfiler _(ProfilePhase::MaterialSample);
        return Dispatch([&](auto&& x) { return x.Le(c); });
    }
};

}  // namespace pine

#endif  // PINE_CORE_MATERIAL_H