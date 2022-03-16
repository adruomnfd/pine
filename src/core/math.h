#ifndef PINE_CORE_MATH_H
#define PINE_CORE_MATH_H

#include <core/defines.h>

#include <cmath>
#include <limits>
#include <cstdint>
#include <string.h>

namespace pine {

static constexpr float Pi = 3.1415926535897f;
static constexpr float Epsilon = std::numeric_limits<float>::epsilon();
static constexpr float OneMinusEpsilon = 0x1.fffffep-1;
static constexpr float FloatMax = std::numeric_limits<float>::max();
static constexpr float Infinity = std::numeric_limits<float>::infinity();

template <typename Dst, typename Src>
Dst Reinterpret(const Src& src) {
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

struct float16unsigned {
    float16unsigned() = default;
    float16unsigned(float val) {
        union {
            float f;
            uint32_t i;
        };
        f = val;
        uint32_t exp_mask = 0b01111111100000000000000000000000;
        uint32_t man_mask = 0b00000000011111111111111111111111;
        if (((i & exp_mask) >> 23) == 0)
            return;
        uint32_t exp = ((i & exp_mask) >> 23) - 127;
        uint32_t man = (i & man_mask) >> (23 - 10);
        bits |= (exp + 31) << 10;
        bits |= man;
    }
    operator float() const {
        if (bits == 0)
            return 0.0f;
        uint16_t exp_mask = 0b1111110000000000;
        uint16_t man_mask = 0b0000001111111111;
        union {
            float f;
            uint32_t i;
        };
        i = 0;
        uint32_t exp = ((bits & exp_mask) >> 10) - 31;
        uint32_t man = (bits & man_mask) << (23 - 10);
        i |= (exp + 127) << 23;
        i |= man;
        return f;
    }
    uint16_t bits = 0;
};

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

}  // namespace pine

#endif  // PINE_CORE_MATH_H