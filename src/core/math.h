#ifndef PINE_CORE_MATH_H
#define PINE_CORE_MATH_H

#include <core/defines.h>

#include <cmath>
#include <limits>
#include <cstdint>
#include <string.h>

namespace pine {

static constexpr float Pi = 3.1415926535897f;
static constexpr float Pi2 = Pi * 2;
static constexpr float Pi4 = Pi * 4;
static constexpr float Epsilon = std::numeric_limits<float>::epsilon();
static constexpr float OneMinusEpsilon = 0x1.fffffep-1;
static constexpr float FloatMax = std::numeric_limits<float>::max();
static constexpr float Infinity = std::numeric_limits<float>::infinity();

template <typename Dst, typename Src>
Dst Bitcast(const Src& src) {
    Dst dst;
    memcpy(&dst, &src, sizeof(Dst));
    return dst;
}

template <typename T>
inline T max(T a, T b) {
    return a > b ? a : b;
}

template <typename T>
inline T min(T a, T b) {
    return a < b ? a : b;
}

template <typename T, typename... Ts>
T max(T a, Ts... rest) {
    return max(a, max(rest...));
}

template <typename T, typename... Ts>
T min(T a, Ts... rest) {
    return min(a, min(rest...));
}

template <typename T>
inline T Mod(T a, T b) {
    T result = a - (a / b) * b;
    return T(result < 0 ? result + b : result);
}

template <typename T>
inline T Clamp(T val, T min, T max) {
    return std::min(std::max(val, min), max);
}

template <typename T, typename U>
inline U Lerp(T t, U a, U b) {
    return a * ((T)1 - t) + b * t;
}

template <typename T>
inline T Sqr(T v) {
    return v * v;
}

template <typename T>
inline T Sign(T v) {
    return v > 0 ? 1 : -1;
}

inline float Fract(float v) {
    v = v - (int)v;     // [-1, 1]
    v = v + 1.0f;       // [ 0, 2]
    return v - (int)v;  // [ 0, 1]
}

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

inline uint32_t SeperateLowestSetBit(uint32_t x) {
    return x & -x;
}

inline int HighestSetBit(uint32_t x) {
    union {
        float f;
        uint32_t i;
    };
    f = (float)x;

    return (0xff & (i >> 23)) - 127;
}
inline int Log2Int(int x) {
    if (x <= 0)
        return 0;
    return HighestSetBit(x);
}

inline int CountTrailingZero(uint32_t x) {
    // return __builtin_ctz(x);
    return HighestSetBit(SeperateLowestSetBit(x));
}

inline uint32_t RoundUpPow2(uint32_t x) {
    x -= 1;
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    x += 1;
    return x;
}

inline float ErfInv(float x) {
    float w, p;
    x = Clamp(x, -.99999f, .99999f);
    w = -std::log((1 - x) * (1 + x));
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
        w = std::sqrt(w) - 3;
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

    return Clamp(first - 1, 0, size - 2);
}

}  // namespace pine

#endif  // PINE_CORE_MATH_H