#ifndef PINE_CORE_GEOMETRY_H
#define PINE_CORE_GEOMETRY_H

#include <core/material.h>
#include <core/medium.h>
#include <core/light.h>
#include <core/ray.h>
#include <util/taggedvariant.h>
#include <util/profiler.h>

#include <vector>

namespace pine {

inline vec3 OffsetRayOrigin(vec3 p, vec3 n) {
    float origin = 1.0f / 32.0f;
    float floatScale = 1.0f / 65536.0f;
    float intScale = 256.0f;
    vec3i of_i = intScale * n;
    vec3 p_i = Bitcast<vec3>(Bitcast<vec3i>(p) + vec3i(p.x < 0 ? -of_i.x : of_i.x,
                                                       p.y < 0 ? -of_i.y : of_i.y,
                                                       p.z < 0 ? -of_i.z : of_i.z));
    return {std::abs(p.x) < origin ? p.x + n.x * floatScale : p_i.x,
            std::abs(p.y) < origin ? p.y + n.y * floatScale : p_i.y,
            std::abs(p.z) < origin ? p.z + n.z * floatScale : p_i.z};
}

struct Interaction {
    bool IsSurfaceInteraction() const {
        return phaseFunction == nullptr;
    }
    bool IsMediumInteraction() const {
        return phaseFunction != nullptr;
    }
    Ray SpawnRayTo(vec3 p2) {
        Ray ray;
        ray.d = Normalize(p2 - p, ray.tmax);
        ray.o = OffsetRayOrigin(p, FaceSameHemisphere(n, ray.d));
        ray.tmin = 0.0f;
        ray.tmax *= 1.0f - 1e-3f;
        ray.medium = GetMedium(ray.d);
        return ray;
    }
    Ray SpawnRay(vec3 wo, float distance) {
        Ray ray;
        ray.d = wo;
        ray.o = OffsetRayOrigin(p, FaceSameHemisphere(n, ray.d));
        ray.tmin = 0.0f;
        ray.tmax = distance * (1.0f - 1e-3f);
        ray.medium = GetMedium(ray.d);
        return ray;
    }
    Ray SpawnRay(vec3 w) {
        Ray ray;
        ray.d = w;
        ray.o = OffsetRayOrigin(p, FaceSameHemisphere(n, ray.d));
        ray.tmin = 0.0f;
        ray.tmax = FloatMax;
        ray.medium = GetMedium(ray.d);
        return ray;
    }
    const Medium* GetMedium(vec3 w) const {
        return Dot(w, n) > 0 ? mediumInterface.outside : mediumInterface.inside;
    }

    Fstring Formatting(Format fmt) const {
        return Fstring(fmt, "[Interaction]p & n & uv &", p, n, uv);
    }

    vec3 p;
    vec3 n;
    vec2 uv;
    vec3 dpdu, dpdv;

    const Material* material = nullptr;
    const Shape* shape = nullptr;
    MediumInterface<const Medium*> mediumInterface;
    const PhaseFunction* phaseFunction = nullptr;
    float bvh = 0.0f;
};

struct RayOctant {
    RayOctant(const Ray& ray)
        : octantx3{(int)std::signbit(ray.d[0]) * 3, (int)std::signbit(ray.d[1]) * 3,
                   (int)std::signbit(ray.d[2]) * 3},
          invDir(SafeRcp(ray.d)),
          negOrgDivDir(-ray.o * invDir) {
    }

    int octantx3[3];
    vec3 invDir, negOrgDivDir;
};

struct AABB {
    AABB() = default;
    AABB(vec3 lower, vec3 upper) : lower(lower), upper(upper){};
    int MaxDim() const {
        vec3 diagonal = upper - lower;
        if (diagonal.x > diagonal.y)
            return diagonal.x > diagonal.z ? 0 : 2;
        else
            return diagonal.y > diagonal.z ? 1 : 2;
    }
    vec3 Centroid() const {
        return (lower + upper) / 2;
    }
    float Centroid(int dim) const {
        return (lower[dim] + upper[dim]) / 2;
    }
    vec3 Diagonal() const {
        return Max(upper - lower, vec3(0.0f));
    }
    vec3 Offset(vec3 p) const;
    float Offset(float p, int dim) const;
    float SurfaceArea() const;
    void Extend(vec3 p);
    void Extend(const AABB& aabb);
    friend AABB Union(AABB l, const AABB& r) {
        l.Extend(r);
        return l;
    }
    friend AABB Intersection(const AABB& l, const AABB& r) {
        AABB ret;
        ret.lower = Max(l.lower, r.lower);
        ret.upper = Min(l.upper, r.upper);
        return ret;
    }
    bool IsValid() const {
        return (upper.x > lower.x) && (upper.y > lower.y) && (upper.z > lower.z);
    }
    bool IsValid(int dim) const {
        return upper[dim] > lower[dim];
    }
    bool IsInside(const AABB& it) {
        return it.lower[0] >= lower[0] && it.lower[1] >= lower[1] && it.lower[2] >= lower[2] &&
               it.upper[0] <= upper[0] && it.upper[1] <= lower[1] && it.upper[2] <= upper[2];
    }
    void CheckIsInside(const AABB& it) {
        CHECK_GE(it.lower[0], lower[0]);
        CHECK_GE(it.lower[1], lower[1]);
        CHECK_GE(it.lower[2], lower[2]);
        CHECK_LE(it.upper[0], upper[0]);
        CHECK_LE(it.upper[1], upper[1]);
        CHECK_LE(it.upper[2], upper[2]);
    }
    Fstring Formatting(Format fmt) const {
        return Fstring(fmt, "lower:& upper:&", lower, upper);
    }

    bool Hit(const Ray& ray) const;
    bool Hit(Ray ray, float& tmin, float& tmax) const;

    bool Hit(vec3 negOrgDivDir, vec3 invdir, float tmin, float tmax) const {
#pragma unroll
        for (int i = 0; i < 3; i++) {
            float t0 = lower[i] * invdir[i] + negOrgDivDir[i];
            float t1 = upper[i] * invdir[i] + negOrgDivDir[i];
            tmin = max(tmin, min(t0, t1));
            tmax = min(tmax, max(t0, t1));
            if (tmin > tmax)
                return false;
        }
        return true;
    }
    bool Hit(vec3 negOrgDivDir, vec3 invdir, float tmin, float* tmax) const {
#pragma unroll
        for (int i = 0; i < 3; i++) {
            float t0 = lower[i] * invdir[i] + negOrgDivDir[i];
            float t1 = upper[i] * invdir[i] + negOrgDivDir[i];
            tmin = max(tmin, min(t0, t1));
            *tmax = min(*tmax, max(t0, t1));
            if (tmin > *tmax)
                return false;
        }
        return true;
    }

    PINE_ALWAYS_INLINE bool Hit(const RayOctant& r, float tmin, float tmax) const {
        const float* p = &lower[0];
        float tmin0 = p[0 + r.octantx3[0]] * r.invDir[0] + r.negOrgDivDir[0];
        float tmin1 = p[1 + r.octantx3[1]] * r.invDir[1] + r.negOrgDivDir[1];
        float tmin2 = p[2 + r.octantx3[2]] * r.invDir[2] + r.negOrgDivDir[2];

        float tmax0 = p[3 - r.octantx3[0]] * r.invDir[0] + r.negOrgDivDir[0];
        float tmax1 = p[4 - r.octantx3[1]] * r.invDir[1] + r.negOrgDivDir[1];
        float tmax2 = p[5 - r.octantx3[2]] * r.invDir[2] + r.negOrgDivDir[2];

        tmin = max(max(max(tmin0, tmin1), tmin2), tmin);
        tmax = min(min(min(tmax0, tmax1), tmax2), tmax);
        return tmin <= tmax;
    }
    PINE_ALWAYS_INLINE bool Hit(const RayOctant& r, float tmin, float* tmax) const {
        const float* p = &lower[0];
        float tmin0 = p[0 + r.octantx3[0]] * r.invDir[0] + r.negOrgDivDir[0];
        float tmin1 = p[1 + r.octantx3[1]] * r.invDir[1] + r.negOrgDivDir[1];
        float tmin2 = p[2 + r.octantx3[2]] * r.invDir[2] + r.negOrgDivDir[2];

        float tmax0 = p[3 - r.octantx3[0]] * r.invDir[0] + r.negOrgDivDir[0];
        float tmax1 = p[4 - r.octantx3[1]] * r.invDir[1] + r.negOrgDivDir[1];
        float tmax2 = p[5 - r.octantx3[2]] * r.invDir[2] + r.negOrgDivDir[2];

        tmin = max(max(max(tmin0, tmin1), tmin2), tmin);
        *tmax = min(min(min(tmax0, tmax1), tmax2), *tmax);
        return tmin <= *tmax;
    }

    vec3 lower = vec3(FloatMax);
    vec3 upper = vec3(-FloatMax);
};

struct ShapeSample : LightSample {
    vec3 p;
    vec3 n;
    vec2 uv;
};

struct Plane {
    static Plane Create(const Parameters& params);
    Plane() = default;
    Plane(vec3 position, vec3 normal) : position(position), n(normal) {
        mat3 tbn = CoordinateSystem(normal);
        u = tbn.x;
        v = tbn.y;
    };

    bool Hit(const Ray& ray) const;
    bool Intersect(Ray& ray, Interaction& it) const;
    AABB GetAABB() const;
    float Area() const {
        return FloatMax;
    }
    ShapeSample Sample(vec3, vec2) const {
        LOG_FATAL("[Plane]doesn't support Sample()");
        return {};
    }

    vec3 position;
    vec3 n;
    vec3 u, v;
};

struct Sphere {
    static Sphere Create(const Parameters& params);
    Sphere() = default;
    Sphere(vec3 position, float radius) : c(position), r(radius){};

    static float ComputeT(vec3 ro, vec3 rd, float tmin, vec3 p, float r);
    bool Hit(const Ray& ray) const;
    bool Intersect(Ray& ray, Interaction& it) const;
    AABB GetAABB() const;
    float Area() const {
        return 4 * Pi * r * r;
    }
    ShapeSample Sample(vec3, vec2 u) const {
        ShapeSample ss;
        ss.n = UniformSphereSampling(u);
        ss.p = c + r * ss.n;
        ss.uv = u * vec2(Pi * 2, Pi);
        return ss;
    }

    PINE_ARCHIVE(c, r)

    vec3 c;
    float r;
};

struct Cylinder {
    static Cylinder Create(const Parameters& params);
    Cylinder() = default;
    Cylinder(vec3 pos, float r, float height, float phiMax)
        : pos(pos), r(r), height(height), phiMax(phiMax){};

    bool Hit(const Ray& ray) const;
    bool Intersect(Ray& ray, Interaction& it) const;
    AABB GetAABB() const;
    float Area() const {
        return height * Pi * 2 * r;
    }
    ShapeSample Sample(vec3, vec2) const {
        LOG_FATAL("[Cylinder]doesn't support Sample()");
        return {};
    }

    vec3 pos;
    float r;
    float height;
    float phiMax;
};

struct Disk {
    static Disk Create(const Parameters& params);
    Disk() = default;
    Disk(vec3 position, vec3 normal, float r) : position(position), n(normal), r(r) {
        mat3 tbn = CoordinateSystem(normal);
        u = tbn.x;
        v = tbn.y;
    };

    bool Hit(const Ray& ray) const;
    bool Intersect(Ray& ray, Interaction& it) const;
    AABB GetAABB() const;
    float Area() const {
        return Pi * r * r;
    }
    ShapeSample Sample(vec3, vec2) const {
        LOG_FATAL("[Disk]doesn't support Sample()");
        return {};
    }

    vec3 position;
    vec3 n;
    vec3 u, v;
    float r;
};

struct Line {
    static Line Create(const Parameters& params);
    Line() = default;
    Line(vec3 p0, vec3 p1, float thickness) : p0(p0), p1(p1), thickness(thickness){};

    bool Hit(const Ray& ray) const;
    bool Intersect(Ray& ray, Interaction& it) const;
    AABB GetAABB() const;
    float Area() const {
        return 0;
    }
    ShapeSample Sample(vec3, vec2) const {
        LOG_FATAL("[Line]doesn't support Sample()");
        return {};
    }

    vec3 p0, p1;
    float thickness;
};

struct Triangle {
    static Triangle Create(const Parameters& params);
    Triangle() = default;
    Triangle(vec3 v0, vec3 v1, vec3 v2) : v0(v0), v1(v1), v2(v2){};

    bool Hit(const Ray& ray) const {
        return Hit(ray, v0, v1, v2);
    }
    bool Intersect(Ray& ray, Interaction& it) const {
        return Intersect(ray, it, v0, v1, v2);
    }

    static inline bool Hit(const Ray& ray, vec3 v0, vec3 v1, vec3 v2) {
        vec3 E1 = v1 - v0;
        vec3 E2 = v2 - v0;
        vec3 T = ray.o - v0;
        vec3 P = Cross(ray.d, E2);
        vec3 Q = Cross(T, E1);
        float D = Dot(P, E1);
        if (D == 0.0f)
            return false;
        float t = Dot(Q, E2) / D;
        if (t < ray.tmin || t > ray.tmax)
            return false;
        float u = Dot(P, T) / D;
        if (u < 0.0f || u > 1.0f)
            return false;
        float v = Dot(Q, ray.d) / D;
        if (v < 0.0f || v > 1.0f)
            return false;
        return u + v < 1.0f;
    }
    static inline bool Intersect(Ray& ray, Interaction& it, vec3 v0, vec3 v1, vec3 v2) {
        vec3 E1 = v1 - v0;
        vec3 E2 = v2 - v0;
        vec3 T = ray.o - v0;
        vec3 P = Cross(ray.d, E2);
        vec3 Q = Cross(T, E1);
        float D = Dot(P, E1);
        if (D == 0.0f)
            return false;
        float t = Dot(Q, E2) / D;
        if (t <= ray.tmin || t >= ray.tmax)
            return false;
        float u = Dot(P, T) / D;
        if (u < 0.0f || u > 1.0f)
            return false;
        float v = Dot(Q, ray.d) / D;
        if (v < 0.0f || v > 1.0f)
            return false;
        if (u + v > 1.0f)
            return false;
        ray.tmax = t;
        it.uv = vec2(u, v);
        return true;
    }

    AABB GetAABB() const;
    vec3 Vertex(int i) const {
        switch (i) {
        case 0: return v0;
        case 1: return v1;
        case 2: return v2;
        default:
            CHECK(false);
            return v0;
            break;
        }
    }
    vec3 Normal() const {
        return Normalize(Cross(v0 - v1, v0 - v2));
    }
    vec3 InterpolatePosition(vec2 bc) const {
        return (1.0f - bc.x - bc.y) * v0 + bc.x * v1 + bc.y * v2;
    }
    void ComputeDpDuv(vec3& dpdu, vec3& dpdv) const {
        dpdu = v1 - v0;
        dpdv = v2 - v0;
    }
    float Area() const {
        return Length(Cross(v1 - v0, v2 - v0)) / 2;
    }
    ShapeSample Sample(vec3, vec2 u) const {
        ShapeSample ss;
        if (u.x + u.y > 1.0f)
            u = vec2(1.0f) - u;
        ss.p = InterpolatePosition(u);
        ss.n = Normal();
        ss.uv = u;
        return ss;
    }

    PINE_ARCHIVE(v0, v1, v2)

    vec3 v0, v1, v2;
};

struct Rect {
    static Rect Create(const Parameters& params);
    Rect() = default;
    Rect(vec3 position, vec3 ex, vec3 ey)
        : position(position),
          ex(Normalize(ex)),
          ey(Normalize(ey)),
          n(Normalize(Cross(ex, ey))),
          lx(Length(ex)),
          ly(Length(ey)){};

    bool Hit(const Ray& ray) const;
    bool Intersect(Ray& ray, Interaction& it) const;
    AABB GetAABB() const;
    float Area() const {
        return lx * ly;
    }
    ShapeSample Sample(vec3, vec2 u) const {
        ShapeSample ss;
        ss.p = position + ex * lx * (u.x - 0.5f) + ey * ly * (u.y - 0.5f);
        ss.n = n;
        ss.uv = u;
        return ss;
    }

    vec3 position, ex, ey, n;
    float lx, ly;
};

struct TriangleMesh {
    static TriangleMesh Create(const Parameters& params);
    TriangleMesh() = default;
    TriangleMesh(std::vector<vec3> vertices, std::vector<uint32_t> indices)
        : vertices(std::move(vertices)), indices(std::move(indices)){};

    bool Hit(const Ray&) const {
        return false;
    }
    bool Intersect(Ray&, Interaction&) const {
        return false;
    }
    AABB GetAABB() const {
        return {};
    }
    float Area() const {
        return {};
    }
    float Pdf(const Ray&, const Interaction&) const {
        return {};
    }

    int GetNumTriangles() const {
        return indices.size() / 3;
    }
    Triangle GetTriangle(int index) const {
        return {vertices[indices[index * 3 + 0]], vertices[indices[index * 3 + 1]],
                vertices[indices[index * 3 + 2]]};
    }
    std::vector<Triangle> ToTriangles() const {
        std::vector<Triangle> ts(GetNumTriangles());
        for (int i = 0; i < GetNumTriangles(); i++)
            ts[i] = GetTriangle(i);
        return ts;
    }
    ShapeSample Sample(vec3, vec2) const {
        LOG_FATAL("[TriangleMesh]doesn't support Sample()");
        return {};
    }

    std::vector<vec3> vertices;
    std::vector<vec3> normals;
    std::vector<vec2> texcoords;
    std::vector<uint32_t> indices;
};

struct Shape : TaggedVariant<Sphere, Plane, Triangle, Rect, Cylinder, Disk, Line, TriangleMesh> {
    using TaggedVariant::TaggedVariant;
    static Shape Create(const Parameters& params, Scene* scene);

    bool Hit(const Ray& ray) const {
        SampledProfiler _(ProfilePhase::ShapeIntersect);
        return Dispatch([&](auto&& x) { return x.Hit(ray); });
    }
    bool Intersect(Ray& ray, Interaction& it) const {
        SampledProfiler _(ProfilePhase::ShapeIntersect);
        return Dispatch([&](auto&& x) { return x.Intersect(ray, it); });
    }
    const AABB& GetAABB() const {
        return aabb;
    }
    float Area() const {
        return Dispatch([&](auto&& x) { return x.Area(); });
    }
    float Pdf(const Ray& ray, const Interaction& it) const {
        return Sqr(ray.tmax) / (AbsDot(-ray.d, it.n) * Area());
    }
    ShapeSample Sample(vec3 p, vec2 u) const {
        CHECK(material);
        ShapeSample ss = Dispatch([&](auto&& x) { return x.Sample(p, u); });
        ss.wo = Normalize(ss.p - p, ss.distance);
        MaterialEvalContext mc(ss.p, ss.n, ss.uv, vec3(), vec3(), -ss.wo);
        ss.Le = material->Le(mc);
        ss.pdf = Sqr(ss.distance) / (AbsDot(-ss.wo, ss.n) * Area());
        return ss;
    }
    std::optional<Light> GetLight() const {
        if (material && material->Is<EmissiveMaterial>())
            return AreaLight{this};
        else
            return std::nullopt;
    }

    AABB aabb;
    std::shared_ptr<Material> material;
    MediumInterface<std::shared_ptr<Medium>> mediumInterface;
};

}  // namespace pine

#endif  // PINE_CORE_GEOMETRY_H