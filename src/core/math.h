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
static constexpr float OneMinusEpsilon = 1.0f - Epsilon;
static constexpr float FloatMax = std::numeric_limits<float>::max();
static constexpr float Infinity = std::numeric_limits<float>::infinity();

template <typename Dst, typename Src>
Dst Reinterpret(const Src& src) {
    static_assert(sizeof(Src) >= sizeof(Dst),
                  "sizeof _Src_ must be at least as large as sizeof _Dst_");
    Dst dst;
    memcpy(&dst, &src, sizeof(Dst));
    return dst;
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

}  // namespace pine

#endif  // PINE_CORE_MATH_H