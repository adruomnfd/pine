#ifndef PINE_CORE_LIGHT_H
#define PINE_CORE_LIGHT_H

#include <core/vecmath.h>
#include <util/taggedptr.h>

namespace pine {

struct LightSample {
    vec3 p;
    vec3 wo;
    vec3 Le;
    float distance = 1.0f;
    float pdf = 1.0f;
    bool isDelta = false;
};
struct LightEmissionSample {
    vec3 p;
    vec3 wo;
    vec3 Le;
};

struct PointLight {
    static PointLight Create(const Parameters& params);
    PointLight(vec3 position, vec3 color) : position(position), color(color){};

    LightSample Sample(vec3 p, float u1, vec2 u2) const;
    LightEmissionSample SampleEmission(float u1, vec2 up, vec2 ud) const;

    vec3 position;
    vec3 color;
};

struct DirectionalLight {
    static DirectionalLight Create(const Parameters& params);
    DirectionalLight(vec3 direction, vec3 color) : direction(Normalize(direction)), color(color){};

    LightSample Sample(vec3 p, float u1, vec2 u2) const;
    LightEmissionSample SampleEmission(float u1, vec2 up, vec2 ud) const;

    vec3 direction;
    vec3 color;
};

struct AreaLight {
    static AreaLight Create(const Parameters& params);
    AreaLight(vec3 position, vec3 ex, vec3 ey, vec3 color)
        : position(position), ex(ex), ey(ey), n(Normalize(Cross(ex, ey))), color(color){};

    LightSample Sample(vec3 p, float u1, vec2 u2) const;
    LightEmissionSample SampleEmission(float u1, vec2 up, vec2 ud) const;

    vec3 position;
    vec3 ex;
    vec3 ey;
    vec3 n;
    vec3 color;
};

struct MeshLight {
    static MeshLight Create(const Parameters& params, const Scene* scene);
    MeshLight(const TriangleMesh* mesh) : mesh(mesh){};

    LightSample Sample(vec3 p, float u1, vec2 u2) const;
    LightEmissionSample SampleEmission(float u1, vec2 up, vec2 ud) const;

    const TriangleMesh* mesh;
};

struct Light : TaggedPointer<PointLight, DirectionalLight, AreaLight, MeshLight> {
    using TaggedPointer::TaggedPointer;
    static Light Create(const Parameters& params, const Scene* scene);
    static void Destory(Light light) {
        light.Delete();
    }

    LightSample Sample(vec3 p, float u1, vec2 u2) const {
        return Dispatch([=](auto ptr) { return ptr->Sample(p, u1, u2); });
    }
    LightEmissionSample SampleEmission(float u1, vec2 up, vec2 ud) const {
        return Dispatch([=](auto ptr) { return ptr->SampleEmission(u1, up, ud); });
    }
};

}  // namespace pine

#endif