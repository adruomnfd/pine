#ifndef PINE_UTIL_SOBOLMETRICES_H
#define PINE_UTIL_SOBOLMETRICES_H

#include <core/defines.h>

#include <cstdint>

namespace pine {

static constexpr int NSobolDimensions = 1024;
static constexpr int SobolMatrixSize = 52;
extern const uint32_t SobolMatrices32[NSobolDimensions * SobolMatrixSize];
extern const uint64_t VdCSobolMatrices[][SobolMatrixSize];
extern const uint64_t VdCSobolMatricesInv[][SobolMatrixSize];

}  // namespace pine

#endif  // PINE_UTIL_SOBOLMETRICES_H