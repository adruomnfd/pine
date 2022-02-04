#ifndef PINE_CORE_GEOMETRY_H
#define PINE_CORE_GEOMETRY_H

#include <core/vecmath.h>
#include <core/material.h>
#include <core/medium.h>
#include <util/taggedptr.h>
#include <util/profiler.h>

#include <vector>

namespace pine {

struct Ray {
    static Ray SpawnRay(vec3 p0, vec3 p1) {
        Ray ray;
        ray.o = p0;
        ray.d = Normalize(p1 - p0, ray.tmax);
        return ray;
    };

    Ray() = default;
    Ray(vec3 o, vec3 d, float tmin = 0.0f, float tmax = FloatMax)
        : o(o), d(d), tmin(tmin), tmax(tmax){};
    vec3 operator()(float t) const {
        return o + t * d;
    }

    FormattedString Formatting(Format fmt) const {
        return FormattedString(fmt, "[Ray]origin & direction & tmin & tmax &", o, d, tmin, tmax);
    }

    vec3 o;
    vec3 d;
    float tmin = 0.0f;
    float tmax = FloatMax;
    Medium medium;
};

struct Interaction {
    bool IsSurfaceInteraction() const {
        return !isMediumInteraction;
    }
    bool IsMediumInteraction() const {
        return isMediumInteraction;
    }
    Ray SpawnRayTo(vec3 p2) {
        Ray ray;
        ray.o = p;
        ray.d = Normalize(p2 - p, ray.tmax);
        ray.tmax *= 1.0f - 1e-4f;
        ray.tmin = 1e-4f;
        ray.medium = GetMedium(ray.d);
        return ray;
    }
    Ray SpawnRay(vec3 w) {
        Ray ray;
        ray.o = p;
        ray.d = w;
        ray.tmin = 1e-4f;
        ray.medium = GetMedium(ray.d);
        return ray;
    }
    Medium GetMedium(vec3 w) const {
        return Dot(w, n) > 0 ? mediumInterface.outside : mediumInterface.inside;
    }

    vec3 p;
    vec3 n;
    vec2 uv;
    Material material;
    MediumInterface mediumInterface;
    PhaseFunction phaseFunction;
    bool isMediumInteraction = false;
    float bvh = 0.0f;
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
    void Extend(AABB aabb);
    friend AABB Union(AABB l, const AABB& r) {
        l.Extend(r);
        return l;
    }
    friend AABB Intersection(AABB l, AABB r) {
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
    bool IsInside(AABB it) {
        return it.lower[0] >= lower[0] && it.lower[1] >= lower[1] && it.lower[2] >= lower[2] &&
               it.upper[0] <= upper[0] && it.upper[1] <= lower[1] && it.upper[2] <= upper[2];
    }
    void CheckIsInside(AABB it) {
        CHECK_GE(it.lower[0], lower[0]);
        CHECK_GE(it.lower[1], lower[1]);
        CHECK_GE(it.lower[2], lower[2]);
        CHECK_LE(it.upper[0], upper[0]);
        CHECK_LE(it.upper[1], upper[1]);
        CHECK_LE(it.upper[2], upper[2]);
    }
    FormattedString Formatting(Format fmt) const {
        return FormattedString(fmt, "lower:& upper:&", lower, upper);
    }

    bool Hit(Ray ray) const;
    bool Hit(Ray ray, float& tmin, float& tmax) const;

    PINE_ALWAYS_INLINE bool Hit(vec3 negOrgDivDir, vec3 invdir, float tmin, float tmax) const {
#pragma unroll
        for (int i = 0; i < 3; i++) {
            float t0 = lower[i] * invdir[i] + negOrgDivDir[i];
            float t1 = upper[i] * invdir[i] + negOrgDivDir[i];
            tmin = fmaxf(tmin, fminf(t0, t1));
            tmax = fminf(tmax, fmaxf(t0, t1));
            if (tmin > tmax)
                return false;
        }
        return true;
    }
    PINE_ALWAYS_INLINE bool Hit(vec3 negOrgDivDir, vec3 invdir, float tmin, float* tmax) const {
#pragma unroll
        for (int i = 0; i < 3; i++) {
            float t0 = lower[i] * invdir[i] + negOrgDivDir[i];
            float t1 = upper[i] * invdir[i] + negOrgDivDir[i];
            tmin = fmaxf(tmin, fminf(t0, t1));
            *tmax = fminf(*tmax, fmaxf(t0, t1));
            if (tmin > *tmax)
                return false;
        }
        return true;
    }

    PINE_ALWAYS_INLINE bool Hit(int octantx3[3], vec3 negOrgDivDir, vec3 invDir, float tmin,
                                float tmax) const {
        const float* p = &lower[0];
        float tmin0 = p[0 + octantx3[0]] * invDir[0] + negOrgDivDir[0];
        float tmin1 = p[1 + octantx3[1]] * invDir[1] + negOrgDivDir[1];
        float tmin2 = p[2 + octantx3[2]] * invDir[2] + negOrgDivDir[2];

        float tmax0 = p[3 - octantx3[0]] * invDir[0] + negOrgDivDir[0];
        float tmax1 = p[4 - octantx3[1]] * invDir[1] + negOrgDivDir[1];
        float tmax2 = p[5 - octantx3[2]] * invDir[2] + negOrgDivDir[2];

        tmin = fmaxf(fmaxf(fmaxf(tmin0, tmin1), tmin2), tmin);
        tmax = fminf(fminf(fminf(tmax0, tmax1), tmax2), tmax);
        return tmin <= tmax;
    }
    PINE_ALWAYS_INLINE bool Hit(int octantx3[3], vec3 negOrgDivDir, vec3 invDir, float tmin,
                                float* tmax) const {
        const float* p = &lower[0];
        float tmin0 = p[0 + octantx3[0]] * invDir[0] + negOrgDivDir[0];
        float tmin1 = p[1 + octantx3[1]] * invDir[1] + negOrgDivDir[1];
        float tmin2 = p[2 + octantx3[2]] * invDir[2] + negOrgDivDir[2];

        float tmax0 = p[3 - octantx3[0]] * invDir[0] + negOrgDivDir[0];
        float tmax1 = p[4 - octantx3[1]] * invDir[1] + negOrgDivDir[1];
        float tmax2 = p[5 - octantx3[2]] * invDir[2] + negOrgDivDir[2];

        tmin = fmaxf(fmaxf(fmaxf(tmin0, tmin1), tmin2), tmin);
        *tmax = fminf(fminf(fminf(tmax0, tmax1), tmax2), *tmax);
        return tmin <= *tmax;
    }

    vec3 lower = vec3(FloatMax);
    vec3 upper = vec3(-FloatMax);
};

struct Plane {
    static Plane Create(const Parameters& params);
    Plane() = default;
    Plane(vec3 position, vec3 normal) : position(position), n(normal) {
        mat3 tbn = CoordinateSystem(normal);
        u = tbn.x;
        v = tbn.y;
    };

    bool Hit(Ray ray) const;
    bool Intersect(Ray& ray, Interaction& it) const;
    AABB GetAABB() const;

    vec3 position;
    vec3 n;
    vec3 u, v;
};

struct Sphere {
    static Sphere Create(const Parameters& params);
    Sphere() = default;
    Sphere(vec3 center, float radius) : c(center), r(radius){};

    bool Hit(Ray ray) const;
    bool Intersect(Ray& ray, Interaction& it) const;
    AABB GetAABB() const;

    PINE_ARCHIVE(c, r)

    vec3 c;
    float r;
};

struct Triangle {
    static Triangle Create(const Parameters& params);
    Triangle() = default;
    Triangle(vec3 v0, vec3 v1, vec3 v2) : v0(v0), v1(v1), v2(v2) {
    };

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

    static inline bool Hit(const Ray& ray, vec3 v0, vec3 crossE1E2, vec3 crossE2N, vec3 crossNE1,
                    float invDet) {
        float co = Dot(crossE1E2, ray.o - v0);
        float cd = Dot(crossE1E2, ray.d);
        float t = -co / cd;
        if (t <= ray.tmin || t >= ray.tmax)
            return false;
        vec3 p = ray(t) - v0;
        float u = Dot(p, crossE2N) * invDet;
        float v = Dot(p, crossNE1) * invDet;
        return u < 0.0f && v >= 0.0f && u <= 1.0f && v <= 1.0f;
    }
    static inline bool Intersect(Ray& ray, Interaction& it, vec3 v0, vec3 crossE1E2, vec3 crossE2N,
                          vec3 crossNE1, float invDet) {
        float co = Dot(crossE1E2, ray.o - v0);
        float cd = Dot(crossE1E2, ray.d);
        float t = -co / cd;
        if (t <= ray.tmin || t >= ray.tmax)
            return false;
        vec3 p = ray(t) - v0;
        float u = Dot(p, crossE2N) * invDet;
        float v = Dot(p, crossNE1) * invDet;
        if (u < 0.0f || v < 0.0f || u > 1.0f || v > 1.0f)
            return false;
        it.uv.x = u;
        it.uv.y = v;
        ray.tmax = t;
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
    vec3 InterpolatePosition(vec2 barycentric) const {
        return (1.0f - barycentric.x - barycentric.y) * v0 + barycentric.x * v1 +
               barycentric.y * v2;
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

    bool Hit(Ray ray) const;
    bool Intersect(Ray& ray, Interaction& it) const;
    AABB GetAABB() const;

    vec3 position, ex, ey, n;
    float lx, ly;
};

struct TriangleMesh {
    TriangleMesh() = default;
    TriangleMesh(std::vector<vec3> vertices, std::vector<uint32_t> indices)
        : vertices(std::move(vertices)), indices(std::move(indices)){};

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

    std::vector<vec3> vertices;
    std::vector<vec3> normals;
    std::vector<vec2> texcoords;
    std::vector<uint32_t> indices;
    mat4 o2w;

    std::string materialName;
    Material material;
    MediumInterface mediumInterface;
};

struct Shape : TaggedPointer<Sphere, Plane, Triangle, Rect> {
    using TaggedPointer::TaggedPointer;
    static Shape Create(const Parameters& params, Scene* scene);
    static void Destory(Shape shape) {
        shape.Delete();
    }

    bool Hit(Ray ray) const;
    bool Intersect(Ray& ray, Interaction& it) const;
    AABB GetAABB() const;

    std::string materialName;
    Material material;
    MediumInterface mediumInterface;
};

}  // namespace pine

#endif  // PINE_CORE_GEOMETRY_H