#ifndef PINE_STD_LIMITS_H
#define PINE_STD_LIMITS_H

#include <pstd/stdint.h>

namespace pstd {

template <typename T>
struct numeric_limits;

template <>
struct numeric_limits<float> {
    static constexpr float max() {
        return 3.40282347e+38f;
    }
    static constexpr float min() {
        return 1.17549435e-38f;
    }
    static constexpr float lowest() {
        return -3.40282347e+38f;
    }
    static constexpr float epsilon() {
        return 1.19209290e-7f;
    }
    static constexpr float infinity() {
        // TODO
        return max();
    }
};

template <>
struct numeric_limits<int32_t> {
    static constexpr int32_t max() {
        return 1 << 30;
    }
    static constexpr int32_t lowest() {
        return -(1 << 30);
    }
};

}  // namespace pstd

#endif  // PINE_STD_LIMITS_H