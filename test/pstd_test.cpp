#include <pstd/algorithm.h>
#include <pstd/iostream.h>
#include <pstd/memory.h>
#include <pstd/string.h>
#include <pstd/math.h>
#include <pstd/map.h>

#include <util/rng.h>
#include <util/format.h>
#include <util/parameters.h>

#include <cmath>

int main() {
    pine::RNG rng;

    for (int i = 0; i < 20; ++i) {
        // float x = -pstd::log(pstd::sqr(rng.Uniformf()) / 5);
        float x = rng.Uniformf() * 2 - 1;
        float y = rng.Uniformf() * 2 - 1;
        double xd = x;
        double yd = y;
        pine::LOG(pine::Format(0, 8, true, false, false, true), "[& &] [& &] [& &]", x, y,
                  std::atan2(y, x), pstd::atan2(y, x),
                  pstd::abs(std::atan2(y, x) - std::atan2(yd, xd)) / std::atan2(yd, xd),
                  pstd::abs(pstd::atan2(y, x) - std::atan2(yd, xd)) / std::atan2(yd, xd));
    }
}