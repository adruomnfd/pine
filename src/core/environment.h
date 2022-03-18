#ifndef PINE_CORE_ENVIRONMENT_H
#define PINE_CORE_ENVIRONMENT_H

#include <core/spectrum.h>

namespace pine {

struct Atmosphere {
    Spectrum Color(vec3 wo);
    vec3 Sample(vec2 u2);
};

}  // namespace pine

#endif  // PINE_CORE_ENVIRONMENT_H