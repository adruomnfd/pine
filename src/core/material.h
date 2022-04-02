#ifndef PINE_CORE_MATERIAL_H
#define PINE_CORE_MATERIAL_H

#include <core/interaction.h>
#include <core/node.h>
#include <core/bxdf.h>
#include <util/taggedvariant.h>
#include <util/profiler.h>

#include <vector>

namespace pine {

struct MaterialEvalCtx : NodeEvalCtx {
    MaterialEvalCtx(vec3 p, vec3 n, vec2 uv, vec3 dpdu, vec3 dpdv, vec3 wi, vec3 wo = vec3(0.0f),
                    vec2 u2 = vec2(0.0f), float u1 = 0.0f)
        : NodeEvalCtx(p, n, uv), dpdu(dpdu), dpdv(dpdv), n2w(CoordinateSystem(n)), u2(u2), u1(u1) {
        mat3 w2n = Inverse(n2w);
        this->wi = w2n * wi;
        if (wo != vec3(0.0f))
            this->wo = w2n * wo;
    };
    MaterialEvalCtx(const Interaction& it, vec3 wi, vec3 wo = vec3(0.0f), vec2 u2 = vec2(0.0f),
                    float u1 = 0.0f)
        : MaterialEvalCtx(it.p, it.n, it.uv, it.dpdu, it.dpdv, wi, wo, u2, u1){};

    vec3 wi;
    vec3 wo;
    vec3 dpdu, dpdv;
    mat3 n2w;
    vec2 u2;
    float u1;
};

struct LayeredMaterial {
    LayeredMaterial(const Parameters& params);

    std::optional<BSDFSample> Sample(const MaterialEvalCtx& c) const;
    Spectrum F(const MaterialEvalCtx& c) const;
    float PDF(const MaterialEvalCtx& c) const;
    Spectrum Le(const MaterialEvalCtx&) const {
        return {};
    }

    std::vector<BSDF> bsdfs;
};

struct EmissiveMaterial {
    EmissiveMaterial(const Parameters& params);

    std::optional<BSDFSample> Sample(const MaterialEvalCtx&) const {
        return std::nullopt;
    }
    Spectrum F(const MaterialEvalCtx&) const {
        return {};
    }
    float PDF(const MaterialEvalCtx&) const {
        return {};
    }
    Spectrum Le(const MaterialEvalCtx& c) const {
        if (CosTheta(c.wi) > 0.0f)
            return color.EvalVec3(c);
        else
            return vec3(0.0f);
    }

    NodeInput color;
};

struct Material : public TaggedVariant<LayeredMaterial, EmissiveMaterial> {
  public:
    using TaggedVariant::TaggedVariant;
    static Material Create(const Parameters& params);

    vec3 BumpNormal(const MaterialEvalCtx& c) const;

    std::optional<BSDFSample> Sample(const MaterialEvalCtx& c) const {
        SampledProfiler _(ProfilePhase::MaterialSample);
        return Dispatch([&](auto&& x) {
            std::optional<BSDFSample> bs = x.Sample(c);
            if (bs) {
                bs->wo = c.n2w * bs->wo;
                if (bs->f.IsBlack() || bs->pdf == 0.0f)
                    bs = std::nullopt;
            }
            return bs;
        });
    }

    Spectrum F(const MaterialEvalCtx& c) const {
        SampledProfiler _(ProfilePhase::MaterialSample);
        return Dispatch([&](auto&& x) { return x.F(c); });
    }

    float PDF(const MaterialEvalCtx& c) const {
        SampledProfiler _(ProfilePhase::MaterialSample);
        return Dispatch([&](auto&& x) { return x.PDF(c); });
    }

    Spectrum Le(const MaterialEvalCtx& c) const {
        SampledProfiler _(ProfilePhase::MaterialSample);

        return Dispatch([&](auto&& x) { return x.Le(c); });
    }

  private:
    std::optional<NodeInput> bumpMap;
};

}  // namespace pine

#endif  // PINE_CORE_MATERIAL_H