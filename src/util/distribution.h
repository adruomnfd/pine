#ifndef PINE_UTIL_DISTRIBUTION_H
#define PINE_UTIL_DISTRIBUTION_H

#include <core/math.h>

#include <pstd/vector.h>

namespace pine {

struct Distribution1D {
    Distribution1D() = default;
    Distribution1D(const float* f, int n) : func(f, f + n), cdf(n + 1) {
        cdf[0] = 0;
        float dx = 1.0f / Count();

        for (int i = 1; i < n + 1; i++)
            cdf[i] = cdf[i - 1] + func[i - 1] * dx;

        funcInt = cdf[n];

        if (funcInt != 0)
            for (int i = 0; i < n + 1; i++)
                cdf[i] /= funcInt;
    }
    int Count() const {
        return (int)func.size();
    }

    // return PDF(x)
    float SampleContinuous(float u, float& pdf) const {
        if (funcInt == 0) {
            pdf = 1.0f;
            return u;
        }
        int offset = FindInterval((int)cdf.size(), [&](int i) { return cdf[i] <= u; });
        float du = u - cdf[offset];
        if (cdf[offset + 1] - cdf[offset] > 0)
            du /= cdf[offset + 1] - cdf[offset];
        pdf = func[offset];

        return pstd::min((offset + du) / Count(), OneMinusEpsilon);
    }

    // return P(Xi)
    int SampleDiscrete(float u, float& p) const {
        float pdf;
        int offset = SampleContinuous(u, pdf) * Count();
        CHECK_LT(offset, Count());

        float dx = 1.0f / Count();
        p = pdf * dx;

        return offset;
    }

    pstd::vector<float> func, cdf;
    float funcInt;
};

}  // namespace pine

#endif  // PINE_UTIL_DISTRIBUTION_H