#include <core/bxdf.h>
#include <util/log.h>
#include <util/parameters.h>

namespace pine {

std::optional<BSDFSample> DiffuseBSDF::Sample(vec3 wi, float, vec2 u, const NodeEvalCtx& nc) const {
    BSDFSample bs;

    vec3 wo = CosineWeightedSampling(u);
    if (CosTheta(wi) < 0)
        wo *= -1;
    DCHECK(SameHemisphere(wi, wo));

    bs.wo = wo;
    bs.pdf = AbsCosTheta(bs.wo) / Pi;
    bs.f = albedo.EvalVec3(nc) / Pi;
    return bs;
}

vec3 DiffuseBSDF::F(vec3 wi, vec3 wo, const NodeEvalCtx& nc) const {
    if (!SameHemisphere(wi, wo))
        return vec3(0.0f);
    return albedo.EvalVec3(nc) / Pi;
}
float DiffuseBSDF::PDF(vec3 wi, vec3 wo, const NodeEvalCtx&) const {
    if (!SameHemisphere(wi, wo))
        return Epsilon;
    return AbsCosTheta(wo) / Pi;
}

std::optional<BSDFSample> ConductorBSDF::Sample(vec3 wi, float, vec2 u2,
                                                const NodeEvalCtx& nc) const {
    BSDFSample bs;

    float alpha = Clamp(Sqr(roughness.EvalFloat(nc)), 0.001f, 1.0f);
    bs.isSpecular = alpha < 0.2f;
    TrowbridgeReitzDistribution distrib(alpha, alpha);
    vec3 wm = distrib.SampleWm(wi, u2);

    vec3 wo = Reflect(wi, wm);
    if (!SameHemisphere(wi, wo))
        return std::nullopt;

    vec3 fr = FrSchlick(albedo.EvalVec3(nc), AbsCosTheta(wm));

    bs.wo = wo;
    bs.pdf = distrib.PDF(wi, wm) / (4 * AbsDot(wi, wm));
    bs.f = fr * distrib.D(wm) * distrib.G(wo, wi) / (4 * CosTheta(wi) * CosTheta(wo));

    return bs;
}

vec3 ConductorBSDF::F(vec3 wi, vec3 wo, const NodeEvalCtx& nc) const {
    if (!SameHemisphere(wi, wo))
        return {};

    float alpha = Clamp(Sqr(roughness.EvalFloat(nc)), 0.001f, 1.0f);
    TrowbridgeReitzDistribution distrib(alpha, alpha);

    vec3 wh = Normalize(wi + wo);

    vec3 fr = FrSchlick(albedo.EvalVec3(nc), AbsCosTheta(wh));

    return fr * distrib.D(wh) * distrib.G(wo, wi) / (4 * AbsCosTheta(wo) * AbsCosTheta(wi));
}
float ConductorBSDF::PDF(vec3 wi, vec3 wo, const NodeEvalCtx& nc) const {
    if (!SameHemisphere(wi, wo))
        return {};

    float alpha = Clamp(Sqr(roughness.EvalFloat(nc)), 0.001f, 1.0f);
    TrowbridgeReitzDistribution distrib(alpha, alpha);

    vec3 wh = Normalize(wi + wo);

    return distrib.PDF(wi, wh) / (4 * AbsDot(wi, wh));
}

std::optional<BSDFSample> DielectricBSDF::Sample(vec3 wi, float u1, vec2 u2,
                                                 const NodeEvalCtx& nc) const {
    BSDFSample bs;
    float etap = eta.EvalFloat(nc);
    if (CosTheta(wi) < 0)
        etap = 1.0f / etap;
    float fr = FrDielectric(AbsCosTheta(wi), etap);

    float alpha = Clamp(Sqr(roughness.EvalFloat(nc)), 0.001f, 1.0f);
    bs.isSpecular = alpha < 0.2f;
    TrowbridgeReitzDistribution distrib(alpha, alpha);
    vec3 wm = distrib.SampleWm(wi, u2);

    if (u1 < fr) {
        vec3 wo = Reflect(wi, wm);
        if (!SameHemisphere(wi, wo))
            return std::nullopt;

        bs.wo = wo;
        bs.pdf = fr * distrib.PDF(wi, wm) / (4 * AbsDot(wi, wm));
        bs.f = albedo.EvalVec3(nc) * fr * distrib.D(wm) * distrib.G(wo, wi) /
               (4 * CosTheta(wi) * CosTheta(wo));
    } else {
        vec3 wo;
        if (!Refract(wi, wm, etap, wo))
            return std::nullopt;
        float cosThetaO = CosTheta(wo), cosThetaI = CosTheta(wi);
        bs.wo = wo;
        float denom = Sqr(Dot(wo, wm) + Dot(wi, wm) / etap);
        float dwm_dwo = AbsDot(wo, wm) / denom;
        bs.pdf = (1.0f - fr) * distrib.PDF(wi, wm) * dwm_dwo;
        bs.f = albedo.EvalVec3(nc) * (1.0f - fr) * distrib.D(wm) * distrib.G(wi, wo) *
               fabsf(Dot(wo, wm) * Dot(wi, wm) / denom / cosThetaI / cosThetaO);
    }
    return bs;
}

vec3 DielectricBSDF::F(vec3 wi, vec3 wo, const NodeEvalCtx& nc) const {
    float alpha = Clamp(Sqr(roughness.EvalFloat(nc)), 0.001f, 1.0f);
    TrowbridgeReitzDistribution distrib(alpha, alpha);

    float cosThetaO = CosTheta(wo), cosThetaI = CosTheta(wi);
    bool reflect = cosThetaI * cosThetaO > 0;
    float etap = eta.EvalFloat(nc);
    if (!reflect)
        etap = 1.0f / etap;

    vec3 wm = FaceForward(Normalize(wi * etap + wo), vec3(0, 0, 1));
    if (Dot(wm, wo) * cosThetaI < 0.0f || Dot(wm, wi) * cosThetaO < 0.0f)
        return {};

    float fr = FrDielectric(AbsCosTheta(wi), eta.EvalFloat(nc));

    if (reflect) {
        return albedo.EvalVec3(nc) * fr * distrib.D(wm) * distrib.G(wo, wi) /
               (4 * cosThetaI * cosThetaO);
    } else {
        float denom = Sqr(Dot(wo, wm) + Dot(wi, wm) / etap) * cosThetaI * cosThetaO;
        return albedo.EvalVec3(nc) * (1.0f - fr) * distrib.D(wm) * distrib.G(wi, wo) *
               fabsf(Dot(wo, wm) * Dot(wi, wm) / denom);
    }
}
float DielectricBSDF::PDF(vec3 wi, vec3 wo, const NodeEvalCtx& nc) const {
    float alpha = Clamp(Sqr(roughness.EvalFloat(nc)), 0.001f, 1.0f);
    TrowbridgeReitzDistribution distrib(alpha, alpha);

    float cosThetaO = CosTheta(wo), cosThetaI = CosTheta(wi);
    bool reflect = cosThetaI * cosThetaO > 0;
    float etap = eta.EvalFloat(nc);
    if (!reflect)
        etap = 1.0f / etap;

    vec3 wm = FaceForward(Normalize(wi * etap + wo), vec3(0, 0, 1));
    if (Dot(wm, wo) * cosThetaI < 0.0f || Dot(wm, wi) * cosThetaO < 0.0f)
        return {};

    float fr = FrDielectric(AbsCosTheta(wi), eta.EvalFloat(nc));

    if (reflect) {
        return fr * distrib.PDF(wi, wm) / (4 * AbsDot(wi, wm));
    } else {
        float denom = Sqr(Dot(wo, wm) + Dot(wi, wm) / etap);
        float dwm_dwo = AbsDot(wo, wm) / denom;
        return (1.0f - fr) * distrib.PDF(wi, wm) * dwm_dwo;
    }
}

DiffuseBSDF DiffuseBSDF::Create(const Parameters& params) {
    return DiffuseBSDF(Node::Create(params["albedo"]));
}

ConductorBSDF ConductorBSDF::Create(const Parameters& params) {
    return ConductorBSDF(Node::Create(params["albedo"]), Node::Create(params["roughness"]));
}

DielectricBSDF DielectricBSDF::Create(const Parameters& params) {
    NodeInput albedo;
    if (params.HasSubset("albedo"))
        albedo = Node::Create(params["albedo"]);
    else
        albedo = new nodes::Constant(1.0f, vec3(1.0f));
    return DielectricBSDF(albedo, Node::Create(params["roughness"]), Node::Create(params["eta"]));
}

BSDF BSDF::Create(const Parameters& params) {
    std::string type = params.GetString("type");
    SWITCH(type) {
        CASE("Diffuse") return DiffuseBSDF::Create(params);
        CASE("Dielectric") return DielectricBSDF::Create(params);
        CASE("Conductor") return ConductorBSDF::Create(params);
        DEFAULT {
            LOG_WARNING("[BSDF][Create]Unknown type \"&\"", type);
            return DiffuseBSDF::Create(params);
        }
    }
}

}  // namespace pine