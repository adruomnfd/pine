#include <core/geometry.h>
#include <core/scene.h>
#include <util/parameters.h>
#include <util/fileio.h>
#include <util/misc.h>

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
void AABB::Extend(const AABB& aabb) {
    lower = Min(lower, aabb.lower);
    upper = Max(upper, aabb.upper);
    if (!IsValid()) {
        lower = Min(lower, lower - vec3(1e-4f));
        upper = Max(upper, upper + vec3(1e-4f));
    }
}

bool AABB::Hit(const Ray& ray) const {
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

bool Plane::Hit(const Ray& ray) const {
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
    it.p = position + it.uv.x * u + it.uv.y * v;
    it.dpdu = u;
    it.dpdv = v;
    return true;
}
AABB Plane::GetAABB() const {
    return {vec3(-1e+6f), vec3(1e+6f)};
}

bool Sphere::Hit(const Ray& ray) const {
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
    it.n = Normalize(ray(t) - this->c);
    it.p = this->c + it.n * r;
    auto [phi, theta] = CartesianToSpherical(it.n);
    float sinTheta = std::sin(theta), cosTheta = std::cos(theta);
    float sinPhi = std::sin(phi), cosPhi = std::cos(phi);
    it.dpdu = vec3(sinTheta * -sinPhi, sinTheta * cosPhi, 0.0f);
    it.dpdv = vec3(cosTheta * cosPhi, cosTheta * sinPhi, -sinTheta);
    it.uv = vec2(phi, theta);
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

bool Rect::Hit(const Ray& ray) const {
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
    it.p = position + ex * u + ey * v;
    it.n = n;
    it.uv = vec2(u, v);
    it.dpdu = ex;
    it.dpdv = ey;
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

bool Cylinder::Hit(const Ray& ray) const {
    vec3 o = ray.o - pos, d = ray.d;
    float a = Sqr(d.x) + Sqr(d.z);
    float b = 2 * (o.x * d.x + o.z * d.z);
    float c = Sqr(o.x) + Sqr(o.z) - Sqr(r);
    float determinant = b * b - 4 * a * c;
    if (determinant <= 0)
        return false;
    determinant = std::sqrt(determinant);

    float t = (-b - determinant) / (2 * a);
    vec3 ip = ray(t) - pos;
    if (t < ray.tmin || ip.y < 0 || ip.y > height || Phi2pi(ip.x, ip.z) > phiMax)
        t = (-b + determinant) / (2 * a);
    ip = ray(t) - pos;
    if (t < ray.tmin || t > ray.tmax || ip.y < 0 || ip.y > height || Phi2pi(ip.x, ip.z) > phiMax)
        return false;
    return true;
}
bool Cylinder::Intersect(Ray& ray, Interaction& it) const {
    vec3 o = ray.o - pos, d = ray.d;
    float a = Sqr(d.x) + Sqr(d.z);
    float b = 2 * (o.x * d.x + o.z * d.z);
    float c = Sqr(o.x) + Sqr(o.z) - Sqr(r);
    float determinant = b * b - 4 * a * c;
    if (determinant <= 0)
        return false;
    determinant = std::sqrt(determinant);

    float t = (-b - determinant) / (2 * a);
    vec3 ip = ray(t) - pos;
    if (t < ray.tmin || ip.y < 0 || ip.y > height || Phi2pi(ip.x, ip.z) > phiMax)
        t = (-b + determinant) / (2 * a);
    ip = ray(t) - pos;
    if (t < ray.tmin || t > ray.tmax || ip.y < 0 || ip.y > height || Phi2pi(ip.x, ip.z) > phiMax)
        return false;

    ray.tmax = t;
    it.p = ray(t);
    it.n = Normalize(vec3(ip.x, 0.0f, ip.z));
    it.uv = vec2(Phi2pi(ip.x, ip.z) / (Pi * 2), ip.y / height);

    return true;
}
AABB Cylinder::GetAABB() const {
    return {pos - vec3(r, r, 0.0f), pos + vec3(r, r, height)};
}

bool Disk::Hit(const Ray& ray) const {
    float t = (Dot(position, n) - Dot(ray.o, n)) / Dot(ray.d, n);
    if (t < ray.tmin)
        return false;
    if (t >= ray.tmax)
        return false;
    vec3 p = ray.o + t * ray.d - position;
    if (LengthSquared(p) > Sqr(r))
        return false;
    return true;
}
bool Disk::Intersect(Ray& ray, Interaction& it) const {
    float t = (Dot(position, n) - Dot(ray.o, n)) / Dot(ray.d, n);
    if (t < ray.tmin)
        return false;
    if (t >= ray.tmax)
        return false;
    vec3 p = ray.o + t * ray.d - position;
    if (LengthSquared(p) > Sqr(r))
        return false;

    ray.tmax = t;
    it.p = ray.o + t * ray.d;
    it.n = n;
    it.uv = vec2(Length(p), Phi2pi(p.x, p.z));
    it.dpdu = p / it.uv.x;
    it.dpdv = vec3(std::cos(it.uv.y), 0.0f, std::sin(it.uv.y));
    return true;
}

AABB Disk::GetAABB() const {
    return {position - vec3(r, 0.0f, r), position + vec3(r, 0.0f, r)};
}

bool Line::Hit(const Ray& ray) const {
    (void)ray;
    return false;
}
bool Line::Intersect(Ray& ray, Interaction& it) const {
    mat4 r2o = LookAt(ray.o, ray.o + ray.d);
    mat4 o2r = Inverse(r2o);
    vec3 p0 = o2r * vec4(this->p0, 1.0f);
    vec3 p1 = o2r * vec4(this->p1, 1.0f);

    vec3 o = p0;
    vec3 d = p1 - p0;
    vec2 tz = Inverse(mat2(Dot(d, d), -d.z, -d.z, 1.0f)) * vec2(-Dot(o, d), o.z);
    float t = Clamp(tz.x, 0.0f, 1.0f);
    float z = Clamp(o.z + t * d.z, ray.tmin + thickness, ray.tmax);

    float D = Length(o + t * d - vec3(0.0f, 0.0f, z));

    if (D > thickness)
        return false;

    ray.tmax = z;
    it.p = ray(z);
    it.n = -ray.d;

    return true;
}
AABB Line::GetAABB() const {
    AABB aabb;
    aabb.Extend(p0 - vec3(thickness));
    aabb.Extend(p1 - vec3(thickness));
    aabb.Extend(p0 + vec3(thickness));
    aabb.Extend(p1 + vec3(thickness));
    return aabb;
}

Sphere Sphere::Create(const Parameters& params) {
    return Sphere(params.GetVec3("position"), params.GetFloat("radius"));
}

Line Line::Create(const Parameters& params) {
    return Line(params.GetVec3("p0"), params.GetVec3("p1"), params.GetFloat("thickness"));
}

Cylinder Cylinder::Create(const Parameters& params) {
    return Cylinder(params.GetVec3("position"), params.GetFloat("radius"),
                    params.GetFloat("height"), params.GetFloat("phiMax"));
}

Disk Disk::Create(const Parameters& params) {
    return Disk(params.GetVec3("position"), Normalize(params.GetVec3("normal")),
                params.GetFloat("radius"));
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

TriangleMesh TriangleMesh::Create(const Parameters& params) {
    return LoadObj(params.GetString("file"));
}

Shape Shape::Create(const Parameters& params, const Scene* scene) {
    std::string type = params.GetString("type");
    Shape shape;

    SWITCH(type) {
        CASE("Sphere") shape = Sphere(Sphere::Create(params));
        CASE("Plane") shape = Plane(Plane::Create(params));
        CASE("Triangle") shape = Triangle(Triangle::Create(params));
        CASE("Rect") shape = Rect(Rect::Create(params));
        CASE("Cylinder") shape = Cylinder(Cylinder::Create(params));
        CASE("Disk") shape = Disk(Disk::Create(params));
        CASE("Line") shape = Line(Line::Create(params));
        CASE("TriangleMesh") shape = TriangleMesh(TriangleMesh::Create(params));
        DEFAULT {
            LOG_WARNING("[Shape][Create]Unknown type \"&\"", type);
            shape = Sphere(Sphere::Create(params));
        }
    }

    shape.aabb = shape.Dispatch([](auto&& x) { return x.GetAABB(); });
    if (auto material = Find(scene->materials, params.GetString("material")))
        shape.material = *material;
    if (auto mediumInside =
            Find(scene->mediums, params.GetString("mediumInside", params.GetString("medium"))))
        shape.mediumInterface.inside = *mediumInside;
    if (auto mediumOutside = Find(scene->mediums, params.GetString("mediumOutside")))
        shape.mediumInterface.outside = *mediumOutside;
    return shape;
}
}  // namespace pine