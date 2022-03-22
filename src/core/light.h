#ifndef PINE_CORE_LIGHT_H
#define PINE_CORE_LIGHT_H

#include <core/spectrum.h>
#include <util/taggedvariant.h>
#include <util/profiler.h>

#include <vector>

namespace pine {

struct LightSample {
    Spectrum Le;
    vec3 wo;
    float distance = 1.0f;
    float pdf = 1.0f;
    bool isDelta = false;
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

struct Sky {
    static Sky Create(const Parameters& params);
    Sky(vec3 sunDirection, Spectrum sunColor) : sunDirection(Normalize(sunDirection)), sunColor(sunColor) {
    }

    LightSample Sample(vec3 p, vec2 u2) const;
    Spectrum Color(vec3 wo) const;
    float Pdf(vec3 wo) const;

    vec3 sunDirection;
    Spectrum sunColor;
};

struct Atmosphere {
    static Atmosphere Create(const Parameters& params);
    Atmosphere(vec3 sunDirection, Spectrum sunColor, vec2i size, bool interpolate);

    LightSample Sample(vec3 p, vec2 u2) const;
    Spectrum Color(vec3 wo) const;
    float Pdf(vec3 wo) const;

    vec3 sunDirection;
    Spectrum sunColor;
    Spectrum sunSampledColor;
    vec2i size;
    std::vector<vec3> colors;
    bool interpolate = true;
};

struct EnvironmentLight : TaggedVariant<Atmosphere, Sky> {
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
    float Pdf(vec3 wo) const {
        SampledProfiler _(ProfilePhase::SampleEnvLight);
        return Dispatch([&](auto&& x) { return x.Pdf(wo); });
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