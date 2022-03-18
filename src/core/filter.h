#ifndef PINE_CORE_FLITER_H
#define PINE_CORE_FLITER_H

#include <core/vecmath.h>
#include <util/taggedvariant.h>

namespace pine {

struct FilterBase {
    FilterBase(vec2 radius) : radius(radius), invRadius(1.0 / radius) {
    }
    vec2 radius, invRadius;
};

struct BoxFilter : FilterBase {
    using FilterBase::FilterBase;
    float Evaluate(vec2) const {
        return 1.0f;
    }
};

struct TriangleFilter : FilterBase {
    using FilterBase::FilterBase;
    float Evaluate(vec2 p) const {
        return max(radius.x - std::abs(p.x), 0.0f) * max(radius.y - std::abs(p.y), 0.0f);
    }
};

struct GaussianFilter : FilterBase {
    GaussianFilter(vec2 radius, float alpha) : FilterBase(radius), alpha(alpha) {
        expX = std::exp(-alpha * Sqr(radius.x));
        expY = std::exp(-alpha * Sqr(radius.y));
    }
    float Evaluate(vec2 p) const {
        return Gaussian(p.x, expX) * Gaussian(p.y, expY);
    }

    float Gaussian(float d, float expv) const {
        return max(std::exp(-alpha * d * d) - expv, 0.0f);
    }

    float alpha;
    float expX, expY;
};

struct MitchellFilter : FilterBase {
    MitchellFilter(vec2 radius, float B, float C) : FilterBase(radius), B(B), C(C) {
    }

    float Evaluate(vec2 p) const {
        return Mitchell1D(p.x * invRadius.x) * Mitchell1D(p.y / invRadius.y);
    }

    float Mitchell1D(float x) const {
        x = std::abs(2 * x);
        if (x > 1)
            return ((-B - 6 * C) * x * x * x + (6 * B + 30 * C) * x * x + (-12 * B - 48 * C) * x +
                    (8 * B + 24 * C)) *
                   (1.f / 6.f);
        else
            return ((12 - 9 * B - 6 * C) * x * x * x + (-18 + 12 * B + 6 * C) * x * x +
                    (6 - 2 * B)) *
                   (1.f / 6.f);
    }

    float B, C;
};

struct LanczosSincFilter : FilterBase {
    LanczosSincFilter(vec2 radius, float tau) : FilterBase(radius), tau(tau) {
    }

    float Evaluate(vec2 p) const {
        return WindowedSinc(p.x, radius.x) * WindowedSinc(p.y, radius.y);
    }

    float Sinc(float x) const {
        x = std::abs(x);
        if (x < 1e-5f)
            return 1.0f;
        return std::sin(Pi * x) / (Pi * x);
    }

    float WindowedSinc(float x, float radius) const {
        x = std::abs(x);
        if (x > radius)
            return 0;
        float lanczos = Sinc(x / tau);
        return Sinc(x) * lanczos;
    }

    float tau;
};

struct Filter
    : TaggedVariant<BoxFilter, TriangleFilter, GaussianFilter, MitchellFilter, LanczosSincFilter> {
  public:
    using TaggedVariant::TaggedVariant;
    static Filter Create(const Parameters& params);

    float Evaluate(vec2 p) const {
        return Dispatch([&](auto&& x) { return x.Evaluate(p); });
    }
    vec2 Radius() const {
        return Dispatch([&](auto&& x) { return x.radius; });
    }
};

}  // namespace pine

#endif  // PINE_CORE_FLITER_H