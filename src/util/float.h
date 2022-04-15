#ifndef PINE_UTIL_FLOAT_H
#define PINE_UTIL_FLOAT_H

#include <core/math.h>

#include <functional>

namespace pine {

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

template <>
struct pstd::hash<pine::float16unsigned> {
    size_t operator()(pine::float16unsigned v) const {
        return pstd::hash<uint16_t>()(v.bits);
    }
};

#endif  // PINE_UTIL_FLOAT_H