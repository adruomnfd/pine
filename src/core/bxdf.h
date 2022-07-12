#ifndef PINE_CORE_BXDF_H
#define PINE_CORE_BXDF_H

#include <core/node.h>
#include <core/spectrum.h>
#include <util/taggedvariant.h>

#include <pstd/optional.h>

namespace pine {

struct BSDFSample {
    vec3 wo;
    Spectrum f = Spectrum(0.5f);
    float pdf = 1.0f;
    bool isSpecular = false;
};

struct DiffuseBSDF {
    static DiffuseBSDF Create(const Parameters& params);
    DiffuseBSDF(NodeInput albedo) : albedo(albedo) {
    }

    pstd::optional<BSDFSample> Sample(vec3 wi, float u1, vec2 u, const NodeEvalCtx& nc) const;
    vec3 F(vec3 wi, vec3 wo, const NodeEvalCtx& nc) const;
    float PDF(vec3 wi, vec3 wo, const NodeEvalCtx& nc) const;

    NodeInput albedo;
};

struct ConductorBSDF {
    static ConductorBSDF Create(const Parameters& params);
    ConductorBSDF(NodeInput albedo, NodeInput roughness) : albedo(albedo), roughness(roughness){};

    pstd::optional<BSDFSample> Sample(vec3 wi, float u1, vec2 u2, const NodeEvalCtx& nc) const;
    vec3 F(vec3 wi, vec3 wo, const NodeEvalCtx& nc) const;
    float PDF(vec3 wi, vec3 wo, const NodeEvalCtx& nc) const;

    NodeInput albedo;
    NodeInput roughness;
};

struct DielectricBSDF {
    static DielectricBSDF Create(const Parameters& params);
    DielectricBSDF(NodeInput albedo, NodeInput roughness, NodeInput eta)
        : albedo(albedo), roughness(roughness), eta(eta){};

    pstd::optional<BSDFSample> Sample(vec3 wi, float u1, vec2 u2, const NodeEvalCtx& nc) const;
    vec3 F(vec3 wi, vec3 wo, const NodeEvalCtx& nc) const;
    float PDF(vec3 wi, vec3 wo, const NodeEvalCtx& nc) const;

    NodeInput albedo;
    NodeInput roughness;
    NodeInput eta;
};

class BSDF : public TaggedVariant<DiffuseBSDF, ConductorBSDF, DielectricBSDF> {
  public:
    using TaggedVariant::TaggedVariant;

    pstd::optional<BSDFSample> Sample(vec3 wi, float u1, vec2 u2, const NodeEvalCtx& nc) const {
        return Dispatch([&](auto&& x) { return x.Sample(wi, u1, u2, nc); });
    }
    vec3 F(vec3 wi, vec3 wo, const NodeEvalCtx& nc) const {
        return Dispatch([&](auto&& x) { return x.F(wi, wo, nc); });
    }
    float PDF(vec3 wi, vec3 wo, const NodeEvalCtx& nc) const {
        return Dispatch([&](auto&& x) { return x.PDF(wi, wo, nc); });
    }
};

BSDF CreateBSDF(const Parameters& params);

}  // namespace pine

#endif  // PINE_CORE_BXDF_H