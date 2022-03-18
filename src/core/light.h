#ifndef PINE_CORE_LIGHT_H
#define PINE_CORE_LIGHT_H

#include <core/vecmath.h>
#include <core/spectrum.h>
#include <util/taggedptr.h>

namespace pine {

struct LightSample {
    Spectrum Le;
    vec3 wo;
    float distance = 1.0f;
    float pdf = 1.0f;
};

struct PointLight {
    static PointLight Create(const Parameters& params);
    PointLight(vec3 position, vec3 color)
        : position(position), color(Spectrum(color, SpectrumType::Illuminant)){};

    LightSample Sample(vec3 p, vec2 u2) const;

    vec3 position;
    Spectrum color;
};

struct DirectionalLight {
    static DirectionalLight Create(const Parameters& params);
    DirectionalLight(vec3 direction, vec3 color)
        : direction(Normalize(direction)), color(Spectrum(color, SpectrumType::Illuminant)){};

    LightSample Sample(vec3 p, vec2 u2) const;

    vec3 direction;
    Spectrum color;
};

struct AreaLight {
    static AreaLight Create(const Parameters& params);
    AreaLight(vec3 position, vec3 ex, vec3 ey, vec3 color)
        : position(position),
          ex(ex),
          ey(ey),
          n(Normalize(Cross(ex, ey))),
          area(Length(Cross(ex, ey))),
          color(Spectrum(color, SpectrumType::Illuminant)){};

    LightSample Sample(vec3 p, vec2 u2) const;

    vec3 position;
    vec3 ex;
    vec3 ey;
    vec3 n;
    float area;
    Spectrum color;
};

struct Atmosphere {
    static Atmosphere Create(const Parameters& params);
    Atmosphere(vec3 sunDirection, vec3 sunColor)
        : sunDirection(Normalize(sunDirection)), sunColor(sunColor){};

    LightSample Sample(vec3 p, vec2 u2) const;
    Spectrum Color(vec3 wo) const;

    vec3 sunDirection;
    vec3 sunColor;
};

struct EnvironmentLight : TaggedPointer<Atmosphere> {
    using TaggedPointer::TaggedPointer;
    static EnvironmentLight Create(const Parameters& params);

    LightSample Sample(vec3 p, vec2 u2) const {
        return Dispatch([&](auto ptr) { return ptr->Sample(p, u2); });
    }
    Spectrum Color(vec3 wo) const {
        return Dispatch([&](auto ptr) { return ptr->Color(wo); });
    }
};

struct Light : TaggedPointer<PointLight, DirectionalLight, AreaLight, EnvironmentLight> {
    using TaggedPointer::TaggedPointer;
    static Light Create(const Parameters& params);

    LightSample Sample(vec3 p, vec2 u2) const {
        return Dispatch([=](auto ptr) { return ptr->Sample(p, u2); });
    }
};

}  // namespace pine

#endif