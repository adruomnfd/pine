#ifndef PINE_CORE_SHADERNODE_H
#define PINE_CORE_SHADERNODE_H

#include <core/noise.h>
#include <core/scattering.h>

#include <pstd/vector.h>
#include <pstd/memory.h>
#include <pstd/string.h>

namespace pine {

struct Node;

struct NodeEvalCtx {
    NodeEvalCtx(vec3 p, vec3 n, vec2 uv) : p(p), n(n), uv(uv){};
    vec3 p;
    vec3 n;
    vec2 uv;
};

struct NodeInput {
    NodeInput() = default;

    NodeInput(Node* node) : link(node){};
    NodeInput(float defaultFloat) : defaultFloat(defaultFloat), defaultVec3(defaultFloat){};
    NodeInput(vec3 defaultVec3) : defaultVec3(defaultVec3){};

    float EvalFloat(const NodeEvalCtx& c) const;
    vec3 EvalVec3(const NodeEvalCtx& c) const;

    pstd::shared_ptr<Node> link = nullptr;
    float defaultFloat = 0.0f;
    vec3 defaultVec3 = vec3(0.0f);
};

struct Node {
    virtual ~Node() = default;

    virtual float EvalFloat(const NodeEvalCtx&) const {
        return 0.0f;
    }
    virtual vec3 EvalVec3(const NodeEvalCtx&) const {
        return vec3(0.0f);
    }
};

Node* CreateNode(const Parameters& params);

namespace nodes {

struct Constant : Node {
    Constant() = default;
    Constant(float vFloat, vec3 vVec3 = {}) : vFloat(vFloat), vVec3(vVec3){};

    float EvalFloat(const NodeEvalCtx&) const override {
        return vFloat;
    }
    vec3 EvalVec3(const NodeEvalCtx&) const override {
        return vVec3;
    }

    float vFloat = 0.0f;
    vec3 vVec3 = vec3(0.0f);
};

struct Position : Node {
    vec3 EvalVec3(const NodeEvalCtx& c) const override {
        return c.p;
    }
};

struct Normal : Node {
    vec3 EvalVec3(const NodeEvalCtx& c) const override {
        return c.n;
    }
};

struct TexCoord : Node {
    vec3 EvalVec3(const NodeEvalCtx& c) const override {
        return vec3(c.uv, 0.0f);
    }
};

struct Decompose : Node {
    Decompose() = default;
    Decompose(NodeInput input, int dimension) : input(input), dimension(dimension){};

    float EvalFloat(const NodeEvalCtx& c) const {
        return input.EvalVec3(c)[dimension];
    }
    vec3 EvalVec3(const NodeEvalCtx& c) const {
        return (vec3)input.EvalVec3(c)[dimension];
    }

    NodeInput input;
    int dimension = 0;
};

struct Composite : Node {
    Composite() = default;
    Composite(NodeInput inputX, NodeInput inputY, NodeInput inputZ)
        : inputX(inputX), inputY(inputY), inputZ(inputZ){};

    vec3 EvalVec3(const NodeEvalCtx& c) const {
        return {inputX.EvalFloat(c), inputY.EvalFloat(c), inputZ.EvalFloat(c)};
    }

    NodeInput inputX, inputY, inputZ;
};

struct Add : Node {
    Add() = default;
    Add(NodeInput input, NodeInput factor) : input(input), factor(factor){};

    float EvalFloat(const NodeEvalCtx& c) const override {
        return input.EvalFloat(c) + factor.EvalFloat(c);
    }
    vec3 EvalVec3(const NodeEvalCtx& c) const override {
        return input.EvalVec3(c) + factor.EvalVec3(c);
    }

    NodeInput input;
    NodeInput factor;
};

struct Substract : Node {
    Substract() = default;
    Substract(NodeInput input, NodeInput factor) : input(input), factor(factor){};

    float EvalFloat(const NodeEvalCtx& c) const override {
        return input.EvalFloat(c) - factor.EvalFloat(c);
    }
    vec3 EvalVec3(const NodeEvalCtx& c) const override {
        return input.EvalVec3(c) - factor.EvalVec3(c);
    }

    NodeInput input;
    NodeInput factor;
};

struct Multiply : Node {
    Multiply() = default;
    Multiply(NodeInput input, NodeInput factor) : input(input), factor(factor){};

    float EvalFloat(const NodeEvalCtx& c) const override {
        return input.EvalFloat(c) * factor.EvalFloat(c);
    }
    vec3 EvalVec3(const NodeEvalCtx& c) const override {
        return input.EvalVec3(c) * factor.EvalVec3(c);
    }

    NodeInput input;
    NodeInput factor;
};

struct Divide : Node {
    Divide() = default;
    Divide(NodeInput input, NodeInput factor) : input(input), factor(factor){};

    float EvalFloat(const NodeEvalCtx& c) const override {
        return input.EvalFloat(c) / factor.EvalFloat(c);
    }
    vec3 EvalVec3(const NodeEvalCtx& c) const override {
        return input.EvalVec3(c) / factor.EvalVec3(c);
    }

    NodeInput input;
    NodeInput factor;
};

struct MultiplyAdd : Node {
    MultiplyAdd() = default;
    MultiplyAdd(NodeInput input, NodeInput mulFactor, NodeInput addFactor)
        : input(input), mulFactor(mulFactor), addFactor(addFactor){};

    float EvalFloat(const NodeEvalCtx& c) const override {
        return input.EvalFloat(c) * mulFactor.EvalFloat(c) + addFactor.EvalFloat(c);
    }
    vec3 EvalVec3(const NodeEvalCtx& c) const override {
        return input.EvalVec3(c) * mulFactor.EvalVec3(c) + addFactor.EvalVec3(c);
    }

    NodeInput input;
    NodeInput mulFactor;
    NodeInput addFactor;
};

struct Length : Node {
    Length() = default;
    Length(NodeInput input) : input(input){};

    float EvalFloat(const NodeEvalCtx& c) const override {
        return pine::Length(input.EvalVec3(c));
    }
    vec3 EvalVec3(const NodeEvalCtx& c) const override {
        return (vec3)pine::Length(input.EvalVec3(c));
    }

    NodeInput input;
};

struct Sqr : Node {
    Sqr() = default;
    Sqr(NodeInput input) : input(input){};

    float EvalFloat(const NodeEvalCtx& c) const override {
        return pstd::sqr(input.EvalFloat(c));
    }
    vec3 EvalVec3(const NodeEvalCtx& c) const override {
        return pstd::sqr(input.EvalVec3(c));
    }

    NodeInput input;
};

struct Sqrt : Node {
    Sqrt() = default;
    Sqrt(NodeInput input) : input(input){};

    float EvalFloat(const NodeEvalCtx& c) const override {
        return pstd::sqrt(input.EvalFloat(c));
    }
    vec3 EvalVec3(const NodeEvalCtx& c) const override {
        return pine::Sqrt(input.EvalVec3(c));
    }

    NodeInput input;
};

struct Pow : Node {
    Pow() = default;
    Pow(NodeInput input, NodeInput exp) : input(input), exp(exp){};

    float EvalFloat(const NodeEvalCtx& c) const override {
        return pstd::pow(input.EvalFloat(c), exp.EvalFloat(c));
    }
    vec3 EvalVec3(const NodeEvalCtx& c) const override {
        return pine::Pow(input.EvalVec3(c), exp.EvalFloat(c));
    }

    NodeInput input;
    NodeInput exp;
};

struct Sin : Node {
    Sin() = default;
    Sin(NodeInput input) : input(input){};

    float EvalFloat(const NodeEvalCtx& c) const override {
        return pstd::sin(input.EvalFloat(c));
    }
    vec3 EvalVec3(const NodeEvalCtx& c) const override {
        vec3 v = input.EvalVec3(c);
        return vec3(pstd::sin(v.x), pstd::sin(v.y), pstd::sin(v.z));
    }

    NodeInput input;
};

struct Cos : Node {
    Cos() = default;
    Cos(NodeInput input) : input(input){};

    float EvalFloat(const NodeEvalCtx& c) const override {
        return pstd::cos(input.EvalFloat(c));
    }
    vec3 EvalVec3(const NodeEvalCtx& c) const override {
        vec3 v = input.EvalVec3(c);
        return vec3(pstd::cos(v.x), pstd::cos(v.y), pstd::cos(v.z));
    }

    NodeInput input;
};

struct Tan : Node {
    Tan() = default;
    Tan(NodeInput input) : input(input){};

    float EvalFloat(const NodeEvalCtx& c) const override {
        return pstd::tan(input.EvalFloat(c));
    }
    vec3 EvalVec3(const NodeEvalCtx& c) const override {
        vec3 v = input.EvalVec3(c);
        return vec3(pstd::tan(v.x), pstd::tan(v.y), pstd::tan(v.z));
    }

    NodeInput input;
};

struct Fract : Node {
    Fract() = default;
    Fract(NodeInput input) : input(input){};

    float EvalFloat(const NodeEvalCtx& c) const override {
        return pstd::fract(input.EvalFloat(c));
    }
    vec3 EvalVec3(const NodeEvalCtx& c) const override {
        return pine::Fract(input.EvalVec3(c));
    }

    NodeInput input;
};

struct Checkerboard : Node {
    Checkerboard() = default;
    Checkerboard(NodeInput input, NodeInput frequency) : input(input), frequency(frequency) {
    }

    float EvalFloat(const NodeEvalCtx& c) const override {
        vec3 p = input.EvalVec3(c);
        p *= frequency.EvalFloat(c);
        return float((pstd::fract(p.x) - 0.5f) * (pstd::fract(p.y) - 0.5f) *
                         (pstd::fract(p.z) - 0.5f) >
                     0.0f);
    }
    vec3 EvalVec3(const NodeEvalCtx& c) const override {
        return (vec3)EvalFloat(c);
    }

    NodeInput input;
    NodeInput frequency;
};

struct Noise : Node {
    Noise() = default;
    Noise(NodeInput input, NodeInput frequency, NodeInput octaves)
        : input(input), frequency(frequency), octaves(octaves){};

    float EvalFloat(const NodeEvalCtx& c) const override {
        return Turbulence(input.EvalVec3(c), (int)frequency.EvalFloat(c), octaves.EvalFloat(c));
    }
    vec3 EvalVec3(const NodeEvalCtx& c) const override {
        return (vec3)Turbulence(input.EvalVec3(c), (int)frequency.EvalFloat(c),
                                octaves.EvalFloat(c));
    }

    NodeInput input;
    NodeInput frequency;
    NodeInput octaves;
};

struct Noise3D : Node {
    Noise3D() = default;
    Noise3D(NodeInput input, NodeInput frequency, NodeInput octaves)
        : input(input), frequency(frequency), octaves(octaves){};

    float EvalFloat(const NodeEvalCtx& c) const override {
        return Turbulence(input.EvalVec3(c), (int)frequency.EvalFloat(c), octaves.EvalFloat(c));
    }
    vec3 EvalVec3(const NodeEvalCtx& c) const override {
        return Turbulence3D(input.EvalVec3(c), (int)frequency.EvalFloat(c), octaves.EvalFloat(c));
    }

    NodeInput input;
    NodeInput frequency;
    NodeInput octaves;
};

struct Reflect : Node {
    Reflect() = default;
    Reflect(NodeInput wi, NodeInput n) : wi(wi), n(n){};

    vec3 EvalVec3(const NodeEvalCtx& c) const override {
        return pine::Reflect(wi.EvalVec3(c), n.EvalVec3(c));
    }

    NodeInput wi;
    NodeInput n;
};

struct Invert : Node {
    Invert() = default;
    Invert(NodeInput input) : input(input){};

    float EvalFloat(const NodeEvalCtx& c) const override {
        return 1.0f - input.EvalFloat(c);
    }
    vec3 EvalVec3(const NodeEvalCtx& c) const override {
        return vec3(1.0f) - input.EvalVec3(c);
    }

    NodeInput input;
};

struct Texture : Node {
    Texture() = default;
    Texture(NodeInput texcoord, pstd::string filename);

    vec3 EvalVec3(const NodeEvalCtx& c) const override;
    float EvalFloat(const NodeEvalCtx& c) const override {
        return EvalVec3(c).x;
    }

    NodeInput texcoord;
    vec2i size;
    pstd::vector<vec3u8> texels;
};

}  // namespace nodes

}  // namespace pine

#endif  // PINE_CORE_SHADERNODE_H