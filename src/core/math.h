#ifndef PINE_CORE_MATH_H
#define PINE_CORE_MATH_H

#include <core/defines.h>

#include <pstd/math.h>
#include <pstd/limits.h>

namespace pine {

static constexpr float Pi = pstd::Pi;
static constexpr float Pi2 = Pi * 2;
static constexpr float Pi4 = Pi * 4;
static constexpr float Epsilon = pstd::numeric_limits<float>::epsilon();
static constexpr float OneMinusEpsilon = 0x1.fffffep-1;
static constexpr float FloatMax = pstd::numeric_limits<float>::max();
static constexpr float Infinity = pstd::numeric_limits<float>::infinity();

inline uint32_t ReverseBits32(uint32_t x) {
    x = (x & 0x55555555) << 1 | (x & 0xaaaaaaaa) >> 1;
    x = (x & 0x33333333) << 2 | (x & 0xcccccccc) >> 2;
    x = (x & 0x0f0f0f0f) << 4 | (x & 0xf0f0f0f0) >> 4;
    x = (x & 0x00ff00ff) << 8 | (x & 0xff00ff00) >> 8;

    return (x << 16) | (x >> 16);
}

inline uint64_t ReverseBits64(uint64_t x) {
    return ((uint64_t)ReverseBits32(x) << 32) | (uint64_t)ReverseBits32(x >> 32);
}

inline uint32_t GrayCode(uint32_t n) {
    return (n >> 1) ^ n;
}

inline float ErfInv(float x) {
    float w, p;
    x = pstd::clamp(x, -.99999f, .99999f);
    w = -pstd::log((1 - x) * (1 + x));
    if (w < 5) {
        w = w - 2.5f;
        p = 2.81022636e-08f;
        p = 3.43273939e-07f + p * w;
        p = -3.5233877e-06f + p * w;
        p = -4.39150654e-06f + p * w;
        p = 0.00021858087f + p * w;
        p = -0.00125372503f + p * w;
        p = -0.00417768164f + p * w;
        p = 0.246640727f + p * w;
        p = 1.50140941f + p * w;
    } else {
        w = pstd::sqrt(w) - 3;
        p = -0.000200214257f;
        p = 0.000100950558f + p * w;
        p = 0.00134934322f + p * w;
        p = -0.00367342844f + p * w;
        p = 0.00573950773f + p * w;
        p = -0.0076224613f + p * w;
        p = 0.00943887047f + p * w;
        p = 1.00167406f + p * w;
        p = 2.83297682f + p * w;
    }
    return p * x;
}

template <typename Predicate>
int FindInterval(int size, const Predicate& pred) {
    if (size < 2)
        return 0;
    int first = 0, len = size;
    while (len > 0) {
        int half = len / 2, middle = first + half;
        if (pred(middle)) {
            first = middle + 1;
            len -= half + 1;
        } else {
            len = half;
        }
    }

    return pstd::clamp(first - 1, 0, size - 2);
}

}  // namespace pine

#endif  // PINE_CORE_MATH_H