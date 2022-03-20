#include <core/bxdf.h>
#include <util/log.h>
#include <util/parameters.h>

namespace pine {

DiffuseBSDF::DiffuseBSDF(const Parameters& params) {
    albedo = Node::Create(params["albedo"]);
}

std::optional<BSDFSample> DiffuseBSDF::Sample(vec3 wi, float, vec2 u, NodeEvalContext nc) const {
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

vec3 DiffuseBSDF::F(vec3 wi, vec3 wo, NodeEvalContext nc) const {
    if (!SameHemisphere(wi, wo))
        return vec3(0.0f);
    return albedo.EvalVec3(nc) / Pi;
}
float DiffuseBSDF::PDF(vec3 wi, vec3 wo, NodeEvalContext) const {
    if (!SameHemisphere(wi, wo))
        return Epsilon;
    return AbsCosTheta(wo) / Pi;
}

ConductorBSDF::ConductorBSDF(const Parameters& params) {
    roughness = Node::Create(params["roughness"]);
    albedo = Node::Create(params["albedo"]);
}

std::optional<BSDFSample> ConductorBSDF::Sample(vec3 wi, float, vec2 u2, NodeEvalContext nc) const {
    BSDFSample bs;

    float alpha = Clamp(Sqr(roughness.EvalFloat(nc)), 0.001f, 1.0f);
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

vec3 ConductorBSDF::F(vec3 wi, vec3 wo, NodeEvalContext nc) const {
    if (!SameHemisphere(wi, wo))
        return {};

    float alpha = Clamp(Sqr(roughness.EvalFloat(nc)), 0.001f, 1.0f);
    TrowbridgeReitzDistribution distrib(alpha, alpha);

    vec3 wh = Normalize(wi + wo);

    vec3 fr = FrSchlick(albedo.EvalVec3(nc), AbsCosTheta(wh));

    return fr * distrib.D(wh) * distrib.G(wo, wi) / (4 * AbsCosTheta(wo) * AbsCosTheta(wi));
}
float ConductorBSDF::PDF(vec3 wi, vec3 wo, NodeEvalContext nc) const {
    if (!SameHemisphere(wi, wo))
        return {};

    float alpha = Clamp(Sqr(roughness.EvalFloat(nc)), 0.001f, 1.0f);
    TrowbridgeReitzDistribution distrib(alpha, alpha);

    vec3 wh = Normalize(wi + wo);

    return distrib.PDF(wi, wh) / (4 * AbsDot(wi, wh));
}

DielectricBSDF::DielectricBSDF(const Parameters& params) {
    roughness = Node::Create(params["roughness"]);
    eta = Node::Create(params["eta"]);
}

std::optional<BSDFSample> DielectricBSDF::Sample(vec3 wi, float u1, vec2 u2,
                                                 NodeEvalContext nc) const {
    BSDFSample bs;
    float fr = FrDielectric(AbsCosTheta(wi), eta.EvalFloat(nc));

    if (u1 < fr) {
        float alpha = Clamp(Sqr(roughness.EvalFloat(nc)), 0.001f, 1.0f);
        TrowbridgeReitzDistribution distrib(alpha, alpha);
        vec3 wm = distrib.SampleWm(wi, u2);

        vec3 wo = Reflect(wi, wm);
        if (!SameHemisphere(wi, wo))
            return std::nullopt;

        bs.wo = wo;
        bs.pdf = fr * distrib.PDF(wi, wm) / (4 * AbsDot(wi, wm));
        bs.f = (vec3)fr * distrib.D(wm) * distrib.G(wo, wi) / (4 * CosTheta(wi) * CosTheta(wo));
    } else {
        vec3 wo;
        if (!Refract(wi, vec3(0, 0, 1), eta.EvalFloat(nc), wo))
            return std::nullopt;
        bs.wo = wo;
        bs.pdf = 1.0f - fr;
        bs.f = vec3(1.0f - fr);
    }
    return bs;
}

vec3 DielectricBSDF::F(vec3 wi, vec3 wo, NodeEvalContext nc) const {
    float alpha = Clamp(Sqr(roughness.EvalFloat(nc)), 0.001f, 1.0f);
    TrowbridgeReitzDistribution distrib(alpha, alpha);

    float cosThetaO = CosTheta(wo), cosThetaI = CosTheta(wi);
    bool reflect = cosThetaI * cosThetaO > 0;
    float etap = 1.0f;
    if (!reflect)
        etap = cosThetaO > 0 ? eta.EvalFloat(nc) : (1.0f / eta.EvalFloat(nc));

    vec3 wm = FaceForward(Normalize(wi * etap + wo), vec3(0, 0, 1));
    if (Dot(wm, wo) * cosThetaI < 0.0f || Dot(wm, wi) * cosThetaO < 0.0f)
        return {};

    float fr = FrDielectric(AbsCosTheta(wi), eta.EvalFloat(nc));

    if (reflect) {
        return (vec3)fr * distrib.D(wm) * distrib.G(wo, wi) / (4 * cosThetaI * cosThetaO);
    } else {
        float denom = Sqr(Dot(wo, wm) + Dot(wi, wm) / etap) * cosThetaI * cosThetaO;
        return (vec3)(1.0f - fr) * distrib.D(wm) * distrib.G(wi, wo) *
               fabsf(Dot(wo, wm) * Dot(wi, wm) / denom);
    }
}
float DielectricBSDF::PDF(vec3 wi, vec3 wo, NodeEvalContext nc) const {
    float alpha = Clamp(Sqr(roughness.EvalFloat(nc)), 0.001f, 1.0f);
    TrowbridgeReitzDistribution distrib(alpha, alpha);

    float cosThetaO = CosTheta(wo), cosThetaI = CosTheta(wi);
    bool reflect = cosThetaI * cosThetaO > 0;
    float etap = 1.0f;
    if (!reflect)
        etap = cosThetaO > 0 ? eta.EvalFloat(nc) : (1.0f / eta.EvalFloat(nc));

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

BSDF BSDF::Create(const Parameters& params) {
    std::string type = params.GetString("type");
    SWITCH(type) {
        CASE("Diffuse") return DiffuseBSDF(params);
        CASE("Dielectric") return DielectricBSDF(params);
        CASE("Conductor") return ConductorBSDF(params);
        DEFAULT {
            LOG_WARNING("[BSDF][Create]Unknown type \"&\"", type);
            return DiffuseBSDF(params);
        }
    }
}

}  // namespace pine