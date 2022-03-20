#ifndef PINE_CORE_BXDF_H
#define PINE_CORE_BXDF_H

#include <core/node.h>
#include <core/spectrum.h>
#include <util/taggedvariant.h>

#include <optional>

namespace pine {

struct BSDFSample {
    vec3 wo;
    Spectrum f = Spectrum(0.5f);
    float pdf = 1.0f;
};

struct DiffuseBSDF {
    DiffuseBSDF(const Parameters& params);
    std::optional<BSDFSample> Sample(vec3 wi, float u1, vec2 u, NodeEvalContext nc) const;
    vec3 F(vec3 wi, vec3 wo, NodeEvalContext nc) const;
    float PDF(vec3 wi, vec3 wo, NodeEvalContext nc) const;

    NodeInput albedo;
};

struct ConductorBSDF {
    ConductorBSDF(const Parameters& params);
    std::optional<BSDFSample> Sample(vec3 wi, float u1, vec2 u2, NodeEvalContext nc) const;
    vec3 F(vec3 wi, vec3 wo, NodeEvalContext nc) const;
    float PDF(vec3 wi, vec3 wo, NodeEvalContext nc) const;

    NodeInput albedo;
    NodeInput roughness;
};

struct DielectricBSDF {
    DielectricBSDF(const Parameters& params);
    std::optional<BSDFSample> Sample(vec3 wi, float u1, vec2 u2, NodeEvalContext nc) const;
    vec3 F(vec3 wi, vec3 wo, NodeEvalContext nc) const;
    float PDF(vec3 wi, vec3 wo, NodeEvalContext nc) const;

    NodeInput roughness;
    NodeInput eta;
};

class BSDF : public TaggedVariant<DiffuseBSDF, ConductorBSDF, DielectricBSDF> {
  public:
    using TaggedVariant::TaggedVariant;
    static BSDF Create(const Parameters& params);

    std::optional<BSDFSample> Sample(vec3 wi, float u1, vec2 u2, NodeEvalContext nc) const {
        return Dispatch([&](auto&& x) { return x.Sample(wi, u1, u2, nc); });
    }
    vec3 F(vec3 wi, vec3 wo, NodeEvalContext nc) const {
        return Dispatch([&](auto&& x) { return x.F(wi, wo, nc); });
    }
    float PDF(vec3 wi, vec3 wo, NodeEvalContext nc) const {
        return Dispatch([&](auto&& x) { return x.PDF(wi, wo, nc); });
    }
};

}  // namespace pine

#endif  // PINE_CORE_BXDF_H