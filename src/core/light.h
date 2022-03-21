#ifndef PINE_CORE_LIGHT_H
#define PINE_CORE_LIGHT_H

#include <core/spectrum.h>
#include <util/taggedvariant.h>
#include <util/profiler.h>

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
    LightSample Sample(vec3 p, vec2 u2) const;

    const Shape* shape = nullptr;
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

struct EnvironmentLight : TaggedVariant<Atmosphere> {
    using TaggedVariant::TaggedVariant;
    static EnvironmentLight Create(const Parameters& params);

    LightSample Sample(vec3 p, vec2 u2) const {
        SampledProfiler _(ProfilePhase::SampleEnvLight);
        return Dispatch([&](auto&& x) { return x.Sample(p, u2); });
    }
    Spectrum Color(vec3 wo) const {
        SampledProfiler _(ProfilePhase::SampleEnvLight);
        return Dispatch([&](auto&& x) { return x.Color(wo); });
    }
};

struct Light : TaggedVariant<PointLight, DirectionalLight, AreaLight, EnvironmentLight> {
    using TaggedVariant::TaggedVariant;
    static Light Create(const Parameters& params);

    LightSample Sample(vec3 p, vec2 u2) const {
        return Dispatch([&](auto&& x) { return x.Sample(p, u2); });
    }
};

}  // namespace pine

#endif