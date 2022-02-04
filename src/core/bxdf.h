#ifndef PINE_CORE_BXDF_H
#define PINE_CORE_BXDF_H

#include <core/node.h>
#include <util/taggedptr.h>

namespace pine {

struct BSDFSample {
    vec3 wo;
    vec3 f = vec3(0.5f);
    float pdf = 1.0f;
};

struct DiffuseBSDF {
    DiffuseBSDF(const Parameters& params);
    BSDFSample Sample(vec3 wi, float u1, vec2 u, NodeEvalContext nc) const;
    vec3 F(vec3 wi, vec3 wo, NodeEvalContext nc) const;
    float PDF(vec3 wi, vec3 wo, NodeEvalContext nc) const;

    NodeInput albedo;
};

struct ConductorBSDF {
    ConductorBSDF(const Parameters& params);
    BSDFSample Sample(vec3 wi, float u1, vec2 u2, NodeEvalContext nc) const;
    vec3 F(vec3 wi, vec3 wo, NodeEvalContext nc) const;
    float PDF(vec3 wi, vec3 wo, NodeEvalContext nc) const;

    NodeInput albedo;
    NodeInput roughness;
};

struct DielectricBSDF {
    DielectricBSDF(const Parameters& params);
    BSDFSample Sample(vec3 wi, float u1, vec2 u2, NodeEvalContext nc) const;
    vec3 F(vec3 wi, vec3 wo, NodeEvalContext nc) const;
    float PDF(vec3 wi, vec3 wo, NodeEvalContext nc) const;

    NodeInput roughness;
    NodeInput eta;
};

class BSDF : public TaggedPointer<DiffuseBSDF, ConductorBSDF, DielectricBSDF> {
  public:
    using TaggedPointer::TaggedPointer;
    static BSDF Create(const Parameters& params);
    static void Destory(BSDF bsdf) {
        bsdf.Delete();
    }

    BSDFSample Sample(vec3 wi, float u1, vec2 u2, NodeEvalContext nc) const;
    vec3 F(vec3 wi, vec3 wo, NodeEvalContext nc) const;
    float PDF(vec3 wi, vec3 wo, NodeEvalContext nc) const;
};

}  // namespace pine

#endif  // PINE_CORE_BXDF_H