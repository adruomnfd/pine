#include <core/geometry.h>
#include <core/scene.h>
#include <util/parameters.h>
#include <util/fileio.h>

namespace pine {

vec3 AABB::Offset(vec3 p) const {
    vec3 o = p - lower;
    if (upper.x > lower.x)
        o.x /= upper.x - lower.x;
    if (upper.y > lower.y)
        o.y /= upper.y - lower.y;
    if (upper.z > lower.z)
        o.z /= upper.z - lower.z;
    return o;
}
float AABB::Offset(float p, int dim) const {
    float o = p - lower[dim];
    float d = upper[dim] - lower[dim];
    return d > 0.0f ? o / d : o;
}
float AABB::SurfaceArea() const {
    vec3 d = Diagonal();
    return 2.0f * (d.x * d.y + d.x * d.z + d.y * d.z);
}
void AABB::Extend(vec3 p) {
    lower = Min(lower, p);
    upper = Max(upper, p);
    if (!IsValid()) {
        lower = Min(lower, lower - vec3(1e-4f));
        upper = Max(upper, upper + vec3(1e-4f));
    }
}
void AABB::Extend(AABB aabb) {
    lower = Min(lower, aabb.lower);
    upper = Max(upper, aabb.upper);
    if (!IsValid()) {
        lower = Min(lower, lower - vec3(1e-4f));
        upper = Max(upper, upper + vec3(1e-4f));
    }
}

bool AABB::Hit(Ray ray) const {
    float tmin = ray.tmin;
    float tmax = ray.tmax;
    if (tmin > tmax)
        return false;
    for (int i = 0; i < 3; i++) {
        float invRayDir = 1.0f / ray.d[i];
        float tNear = (lower[i] - ray.o[i]) * invRayDir;
        float tFar = (upper[i] - ray.o[i]) * invRayDir;
        if (ray.d[i] < 0.0f) {
            float temp = tNear;
            tNear = tFar;
            tFar = temp;
        }
        tmin = tNear > tmin ? tNear : tmin;
        tmax = tFar < tmax ? tFar : tmax;
        if (tmin > tmax)
            return false;
    }
    return true;
}
bool AABB::Hit(Ray ray, float& tmin, float& tmax) const {
    tmin = ray.tmin;
    tmax = ray.tmax;
    if (tmin > tmax)
        return false;
    for (int i = 0; i < 3; i++) {
        float invRayDir = 1.0f / ray.d[i];
        float tNear = (lower[i] - ray.o[i]) * invRayDir;
        float tFar = (upper[i] - ray.o[i]) * invRayDir;
        if (ray.d[i] < 0.0f) {
            float temp = tNear;
            tNear = tFar;
            tFar = temp;
        }
        tmin = tNear > tmin ? tNear : tmin;
        tmax = tFar < tmax ? tFar : tmax;
        if (tmin > tmax)
            return false;
    }
    return true;
}

bool Plane::Hit(Ray ray) const {
    float t = (Dot(position, n) - Dot(ray.o, n)) / Dot(ray.d, n);
    if (t <= ray.tmin)
        return false;
    return t < ray.tmax;
}
bool Plane::Intersect(Ray& ray, Interaction& it) const {
    float t = (Dot(position, n) - Dot(ray.o, n)) / Dot(ray.d, n);
    if (t < ray.tmin)
        return false;
    if (t >= ray.tmax)
        return false;
    ray.tmax = t;
    it.p = ray.o + t * ray.d;
    it.n = n;
    it.uv = vec2(Dot(it.p - position, u), Dot(it.p - position, v));
    return true;
}
AABB Plane::GetAABB() const {
    return {vec3(-1e+6f), vec3(1e+6f)};
}

bool Sphere::Hit(Ray ray) const {
    float a = Dot(ray.d, ray.d);
    float b = 2 * Dot(ray.o - c, ray.d);
    float c = Dot(ray.o, ray.o) + Dot(this->c, this->c) - 2 * Dot(ray.o, this->c) - r * r;
    float d = b * b - 4 * a * c;
    if (d <= 0.0f)
        return false;
    d = std::sqrt(d);
    float t = (-b - d) / (2 * a);
    if (t < ray.tmin)
        t = (-b + d) / (2 * a);
    if (t < ray.tmin)
        return false;
    return t < ray.tmax;
}
bool Sphere::Intersect(Ray& ray, Interaction& it) const {
    float a = Dot(ray.d, ray.d);
    float b = 2 * Dot(ray.o - c, ray.d);
    float c = Dot(ray.o, ray.o) + Dot(this->c, this->c) - 2 * Dot(ray.o, this->c) - r * r;
    float d = b * b - 4 * a * c;
    if (d <= 0.0f)
        return false;
    d = std::sqrt(d);
    float t = (-b - d) / (2 * a);
    if (t < ray.tmin)
        t = (-b + d) / (2 * a);
    if (t < ray.tmin)
        return false;
    if (t > ray.tmax)
        return false;
    ray.tmax = t;
    it.p = ray.o + t * ray.d;
    it.n = (it.p - this->c) / r;
    it.uv = CartesianToSpherical(it.n) / vec2(Pi * 2, Pi);
    return true;
}
AABB Sphere::GetAABB() const {
    return {c - vec3(r), c + vec3(r)};
}

AABB Triangle::GetAABB() const {
    AABB aabb;
    aabb.Extend(v0);
    aabb.Extend(v1);
    aabb.Extend(v2);
    return aabb;
}

bool Rect::Hit(Ray ray) const {
    float t = (Dot(position, n) - Dot(ray.o, n)) / Dot(ray.d, n);
    if (t < ray.tmin)
        return false;
    if (t >= ray.tmax)
        return false;
    vec3 p = ray.o + t * ray.d;
    float u = Dot(p - position, ex) / lx;
    if (u < -0.5f || u > 0.5f)
        return false;
    float v = Dot(p - position, ey) / ly;
    if (v < -0.5f || v > 0.5f)
        return false;
    return true;
}
bool Rect::Intersect(Ray& ray, Interaction& it) const {
    float t = (Dot(position, n) - Dot(ray.o, n)) / Dot(ray.d, n);
    if (t < ray.tmin)
        return false;
    if (t >= ray.tmax)
        return false;
    vec3 p = ray.o + t * ray.d;
    float u = Dot(p - position, ex) / lx;
    if (u < -0.5f || u > 0.5f)
        return false;
    float v = Dot(p - position, ey) / ly;
    if (v < -0.5f || v > 0.5f)
        return false;
    ray.tmax = t;
    it.p = p;
    it.n = n;
    it.uv = vec2(u, v);
    it.pdf = 1.0f / (lx * ly);
    return true;
}
AABB Rect::GetAABB() const {
    AABB aabb;
    aabb.Extend(position + ex * lx / 2);
    aabb.Extend(position - ex * lx / 2);
    aabb.Extend(position + ey * ly / 2);
    aabb.Extend(position - ey * ly / 2);
    return aabb;
}

bool Shape::Hit(Ray ray) const {
    SampledProfiler _(ProfilePhase::ShapeIntersect);
    return Dispatch([&](auto ptr) { return ptr->Hit(ray); });
}
bool Shape::Intersect(Ray& ray, Interaction& it) const {
    SampledProfiler _(ProfilePhase::ShapeIntersect);
    return Dispatch([&](auto ptr) {
        bool hit = ptr->Intersect(ray, it);
        if (hit) {
            it.material = material;
            it.mediumInterface = mediumInterface.IsMediumTransition() ? mediumInterface
                                                                      : MediumInterface(ray.medium);
        }
        return hit;
    });
}
AABB Shape::GetAABB() const {
    return Dispatch([&](auto ptr) { return ptr->GetAABB(); });
}

Sphere Sphere::Create(const Parameters& params) {
    return Sphere(params.GetVec3("center"), params.GetFloat("radius"));
}

Plane Plane::Create(const Parameters& params) {
    return Plane(params.GetVec3("position"), Normalize(params.GetVec3("normal")));
}

Triangle Triangle::Create(const Parameters& params) {
    return Triangle(params.GetVec3("v0"), params.GetVec3("v1"), params.GetVec3("v2"));
}

Rect Rect::Create(const Parameters& params) {
    return Rect(params.GetVec3("position"), params.GetVec3("ex"), params.GetVec3("ey"));
}

Shape Shape::Create(const Parameters& params, Scene* scene) {
    std::string type = params.GetString("type");
    Shape shape;
    if (type == "Sphere") {
        shape = new Sphere(Sphere::Create(params));
    } else if (type == "Plane") {
        shape = new Plane(Plane::Create(params));
    } else if (type == "Triangle") {
        shape = new Triangle(Triangle::Create(params));
    } else if (type == "Rect") {
        shape = new Rect(Rect::Create(params));
    } else {
        LOG_WARNING("[Shape][Create]Unknown type \"&\"", type);
        shape = new Sphere(Sphere::Create(params));
    }
    shape.material = scene->materials[params.GetString("material")];
    shape.mediumInterface.inside = scene->mediums[params.GetString("mediumInside")];
    shape.mediumInterface.outside = scene->mediums[params.GetString("mediumOutside")];
    return shape;
}
}  // namespace pine