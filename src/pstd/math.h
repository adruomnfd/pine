#ifndef PINE_STD_MATH_H
#define PINE_STD_MATH_H

#include <pstd/type_traits.h>
#include <pstd/stdint.h>
#include <pstd/memory.h>

#include <cmath>

namespace pstd {

using std::isinf;
using std::isnan;

using std::acos;
using std::asin;
using std::atan;
using std::atan2;
using std::cos;
using std::sin;
using std::tan;

inline constexpr float E = 2.7182818284590452354f;
inline constexpr float Log2E = 1.4426950408889634074f;
inline constexpr float Log10E = 0.43429448190325182765f;
inline constexpr float Ln2 = 0.69314718055994530942f;
inline constexpr float Ln10 = 2.30258509299404568402f;
inline constexpr float Pi = 3.14159265358979323846f;
inline constexpr float Sqrt2 = 1.41421356237309504880f;

template <typename T>
inline constexpr T min(const T& a, const T& b) {
    return a < b ? a : b;
}
template <typename T>
inline constexpr T max(const T& a, const T& b) {
    return a > b ? a : b;
}

template <typename T, typename... Ts>
inline constexpr T min(const T& a, const Ts&... b) {
    return pstd::min(a, pstd::min(b...));
}
template <typename T, typename... Ts>
inline constexpr T max(const T& a, const Ts&... b) {
    return pstd::max(a, pstd::max(b...));
}

template <typename T>
inline constexpr T abs(T x) {
    return x > 0 ? x : -x;
}

template <typename T>
inline constexpr T copysign(T mag, T sgn) {
    return sgn > 0 ? mag : -mag;
}
template <typename T>
inline constexpr int signbit(T sgn) {
    return sgn < 0 ? 1 : 0;
}

inline int ieeeexp(float x) {
    return (0xff & (pstd::bitcast<uint32_t>(x) >> 23)) - 127;
}

inline float exp2i(int e) {
    return pstd::bitcast<float>((e + 127) << 23);
}

inline int hsb(uint32_t x) {
    return pstd::ieeeexp((float)x);
}

template <typename T>
inline int ctz(T x) {
    return pstd::hsb(x & -x);
}

template <typename T>
inline constexpr T roundup2(T x) {
    if (x == 0)
        return 1;
    x -= 1;

    for (size_t i = 1; i != sizeof(x) * 8; i <<= 1)
        x |= x >> i;

    return x + 1;
}

template <typename T>
inline T rounddown2(T x) {
    return (T)1 << pstd::hsb(x);
}

template <typename T>
inline constexpr T mod(T a, T b) {
    T r = a - (a / b) * b;
    return r < 0 ? r + b : r;
}

template <typename T>
inline constexpr T clamp(T val, T a, T b) {
    return pstd::min(pstd::max(val, a), b);
}

template <typename T, typename U>
inline constexpr U lerp(T t, U a, U b) {
    return a * ((T)1 - t) + b * t;
}

template <typename T>
inline constexpr T sqr(T v) {
    return v * v;
}

template <typename T>
inline constexpr int sign(T v) {
    return v > 0 ? 1 : -1;
}

template <typename T, typename = enable_if_t<is_floating_point_v<T>>>
inline constexpr T floor(T v) {
    auto i = (corresponding_int_t<T>)v;
    if (v < 0 && i != v)
        i -= 1;
    return i;
}

template <typename T, typename = enable_if_t<is_floating_point_v<T>>>
inline constexpr T ceil(T v) {
    auto i = (corresponding_int_t<T>)v;
    if (v > 0 && i != v)
        i += 1;
    return i;
}

template <typename T, typename = enable_if_t<is_floating_point_v<T>>>
inline constexpr T fract(T v) {
    v = v - (corresponding_int_t<T>)v;
    v = v + (T)1;
    return v - (corresponding_int_t<T>)v;
}

template <typename T>
inline constexpr T absfract(T v) {
    v = pstd::abs(v);
    return v - (corresponding_uint_t<T>)v;
}

template <typename T, typename = enable_if_t<is_floating_point_v<T>>>
inline T sqrt(T y) {
    T x = pstd::exp2i(pstd::ieeeexp(y) / 2);

    x = x / 2 + y / (2 * x);
    x = x / 2 + y / (2 * x);
    x = x / 2 + y / (2 * x);
    x = x / 2 + y / (2 * x);

    return x;
}

template <typename T>
inline constexpr T powi(T x, int e) {
    T y = 1;

    while (e) {
        if (e & 1)
            y *= x;
        x *= x;
        e >>= 1;
    };

    return y;
}

template <typename T>
inline constexpr T pow(T x, T e) {
    int ei = pstd::floor(e);
    e -= ei;
    T y = ei > 0 ? pstd::powi(x, ei) : 1 / pstd::powi(x, -ei);

    for (int i = 0; i < 32; ++i) {
        if (e > 1.0f) {
            y *= x;
            e -= 1.0f;
        }
        e *= 2;
        x = pstd::sqrt(x);
    }

    return y;
}

template <typename T, typename = enable_if_t<is_floating_point_v<T>>>
inline T log2(T y) {
    int log = pstd::ieeeexp(y);

    T logl = 0;
    T logr = 1;
    T expl = pstd::exp2i(logl);
    T expr = pstd::exp2i(logr);
    y /= pstd::exp2i(log);

    for (int i = 0; i < 32; ++i) {
        T logm = (logl + logr) / 2;
        T expm = pstd::sqrt(expl * expr);
        if (expm < y) {
            logl = logm;
            expl = expm;
        } else {
            logr = logm;
            expr = expm;
        }
    }

    return log + (logl + logr) / 2;
}

template <typename T, typename = enable_if_t<is_floating_point_v<T>>>
inline T log(T y) {
    return Ln2 * pstd::log2(y);
}

template <typename T>
inline T log2int(T y) {
    return pstd::hsb(y);
}

template <typename T, typename = enable_if_t<is_floating_point_v<T>>>
inline T exp(T x) {
    T y = 0;

    for (int i = 32; i > 0; --i)
        y = 1 + x * y / i;

    return y;
}

}  // namespace pstd

#endif  // PINE_STD_MATH_H