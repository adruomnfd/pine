#ifndef PINE_CORE_MATERIAL_H
#define PINE_CORE_MATERIAL_H

#include <core/node.h>
#include <core/bxdf.h>
#include <util/taggedptr.h>

#include <vector>

namespace pine {

struct MaterialEvalContext : NodeEvalContext {
    MaterialEvalContext() = default;
    MaterialEvalContext(vec3 p, vec3 n, vec2 uv, vec3 wi, vec3 wo = vec3(0.0f),
                        vec2 u2 = vec2(0.0f), float u1 = 0.0f)
        : NodeEvalContext(p, n, uv), wi(wi), wo(wo), u2(u2), u1(u1){};
    vec3 wi;
    vec3 wo;
    vec2 u2;
    float u1;
};

struct LayeredMaterial {
    LayeredMaterial(const Parameters& params);
    ~LayeredMaterial() {
        for (auto bsdf : bsdfs)
            BSDF::Destory(bsdf);
    }
    PINE_DELETE_COPY_MOVE(LayeredMaterial)

    BSDFSample Sample(MaterialEvalContext c) const;
    vec3 F(MaterialEvalContext c) const;
    float PDF(MaterialEvalContext c) const;
    vec3 Le(MaterialEvalContext) const {
        return {};
    }

    std::vector<BSDF> bsdfs;
};

struct EmissiveMaterial {
    EmissiveMaterial(const Parameters& params);

    BSDFSample Sample(MaterialEvalContext) const {
        return {};
    }
    vec3 F(MaterialEvalContext) const {
        return {};
    }
    float PDF(MaterialEvalContext) const {
        return {};
    }
    vec3 Le(MaterialEvalContext c) const {
        return color.EvalVec3(c);
    }

    NodeInput color;
};

struct Material : public TaggedPointer<LayeredMaterial, EmissiveMaterial> {
  public:
    using TaggedPointer::TaggedPointer;
    static Material Create(const Parameters& params);
    static void Destory(Material material) {
        material.Delete();
    }

    BSDFSample Sample(MaterialEvalContext c) const;
    vec3 F(MaterialEvalContext c) const;
    float PDF(MaterialEvalContext c) const;
    vec3 Le(MaterialEvalContext c) const;
};

}  // namespace pine

#endif  // PINE_CORE_MATERIAL_H