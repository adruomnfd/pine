#include <core/medium.h>
#include <core/scene.h>
#include <core/sampling.h>
#include <util/parameters.h>
#include <util/fileio.h>

namespace pine {

float PhaseHG(float cosTheta, float g) {
    float denom = 1.0f + g * g + 2 * g * cosTheta;
    return (1 - g * g) / (denom * std::sqrt(denom) * Pi * 4);
}

float PhaseFunction::F(vec3 wi, vec3 wo) const {
    return PhaseHG(AbsDot(wi, wo), g);
}
float PhaseFunction::Sample(vec3 wi, vec3& wo, vec2 u2) const {
    float cosTheta;
    if (std::abs(g) < 1e-3f) {
        cosTheta = 1 - 2 * u2[0];
    } else {
        float sqrTerm = (1.0f - g * g) / (1 + g - 2 * g * u2[0]);
        cosTheta = -(1.0f + g * g - sqrTerm * sqrTerm) / (2.0f * g);
    }

    float sinTheta = std::sqrt(1.0f - cosTheta * cosTheta);
    float phi = 2 * Pi * u2[1];
    mat3 m = CoordinateSystem(wi);
    wo = m * vec3(sinTheta * std::cos(phi), sinTheta * std::sin(phi), cosTheta);
    return PhaseHG(cosTheta, g);
}

vec3 HomogeneousMedium::Tr(const Ray& ray, Sampler&) const {
    return Exp(-ray.tmax * sigma_t);
}
MediumSample HomogeneousMedium::Sample(const Ray& ray, Interaction& mi, Sampler& sampler) const {
    MediumSample ms;
    float u1 = sampler.Get1D();
    vec2 u2 = sampler.Get2D();
    int channel = int(u1 * 3);
    u1 = u1 * 3 - int(u1 * 3);
    float dist = -std::log(1.0f - u1) / (sigma_t[channel]);
    float t = std::min(dist, ray.tmax);
    bool sampledMedium = t < ray.tmax;
    if (sampledMedium) {
        mi.p = ray(t);
        mi.isMediumInteraction = true;
        mi.mediumInterface = MediumInterface(Medium(const_cast<HomogeneousMedium*>(this)));
        mi.phaseFunction = PhaseFunction(g);
        PhaseFunction pf(0.0f);
        pf.Sample(-ray.d, ms.wo, u2);
    }

    vec3 tr = Exp(-sigma_t * t);
    vec3 density = sampledMedium ? (sigma_t * tr) : tr;
    float pdf = (density[0] + density[1] + density[2]) / 3.0f;
    ms.tr = sampledMedium ? (tr * sigma_s / pdf) : (tr / pdf);
    return ms;
}

vec3 GridMedium::Tr(const Ray& ray, Sampler& sampler) const {
    vec3 tr = vec3(1.0f);
    float tmin, tmax;
    AABB bound(lower, upper);
    if (!bound.Hit(ray, tmin, tmax))
        return vec3(1.0f);

    float t = tmin;
    while (true) {
        t -= std::log(1.0f - sampler.Get1D()) * invMaxDensity / sigma_t;
        if (t >= tmax)
            break;
        float density = Density(ray(t));
        tr *= 1.0f - std::max(0.0f, density * invMaxDensity);
    }
    return tr;
}
MediumSample GridMedium::Sample(const Ray& ray, Interaction& mi, Sampler& sampler) const {
    MediumSample ms;
    float tmin, tmax;
    AABB bound(lower, upper);
    if (!bound.Hit(ray, tmin, tmax))
        return ms;

    float t = tmin;
    while (true) {
        t -= std::log(1.0f - sampler.Get1D()) * invMaxDensity / sigma_t;
        if (t >= tmax)
            break;
        if (Density(ray(t)) * invMaxDensity > sampler.Get1D()) {
            mi.p = ray(t);
            mi.isMediumInteraction = true;
            mi.mediumInterface = MediumInterface(Medium(const_cast<GridMedium*>(this)));
            mi.phaseFunction = PhaseFunction(g);
            PhaseFunction pf(0.0f);
            pf.Sample(-ray.d, ms.wo, sampler.Get2D());
            ms.tr = sigma_s / sigma_t;
        }
    }
    return ms;
}

float GridMedium::Density(vec3 p) const {
    AABB bound(lower, upper);
    p = bound.Offset(p);
    return D(p * size);
}
float GridMedium::D(vec3i p) const {
    if (p.x < 0 || p.y < 0 || p.z < 0 || p.x >= size.x || p.y >= size.y || p.z >= size.z)
        return 0.0f;
    float d = density[p.x + p.y * size.x + p.z * size.y * size.x];
    return d;
}

GridMedium::GridMedium(vec3 sigma_a, vec3 sigma_s, float g, vec3i size, vec3 lower, vec3 upper,
                       float* density)
    : sigma_a(sigma_a),
      sigma_s(sigma_s),
      sigma_t((sigma_a + sigma_s)[0]),
      g(g),
      size(size),
      lower(lower),
      upper(upper),
      density(density) {
    float maxDensity = 0.0f;
    for (int x = 0; x < size.x; x++)
        for (int y = 0; y < size.y; y++)
            for (int z = 0; z < size.z; z++)
                maxDensity = std::max(D(vec3i(x, y, z)), maxDensity);
    invMaxDensity = 1.0f / maxDensity;
}

HomogeneousMedium HomogeneousMedium::Create(const Parameters& params) {
    return HomogeneousMedium(params.GetVec3("sigma_a"), params.GetVec3("sigma_s"),
                             params.GetFloat("g"));
}

GridMedium GridMedium::Create(const Parameters& params) {
    ScopedFile file(sceneDirectory + params.GetString("file"), std::ios::in);
    vec3i lower;
    vec3i upper;
    file.Read(&lower, sizeof(lower));
    file.Read(&upper, sizeof(upper));
    vec3i size = upper - lower;
    size_t N = (size_t)size.x * size.y * size.z;
    float* density = new float[N];
    file.Read(density, sizeof(density[0]) * N);

    LOG("[Medium]Creating grid medium of size &", size);
    LOG("[Medium]Memory usage: & MB", sizeof(density[0]) * N / 1000000.0);
    return GridMedium(params.GetVec3("sigma_a"), params.GetVec3("sigma_s"), params.GetFloat("g"),
                      size, params.GetVec3("lower"), params.GetVec3("upper"), density);
}

Medium Medium::Create(const Parameters& params) {
    std::string type = params.GetString("type");
    SWITCH(params.GetString("type")) {
        CASE("Homogeneous") return new HomogeneousMedium(HomogeneousMedium::Create(params));
        CASE("Grid") return new GridMedium(GridMedium::Create(params));
        DEFAULT {
            LOG_WARNING("[Medium][Create]Unknown type \"&\"", type);
            return new HomogeneousMedium(HomogeneousMedium::Create(params));
        }
    }
}

}  // namespace pine