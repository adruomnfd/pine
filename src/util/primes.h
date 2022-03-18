#ifndef PINE_UTIL_PRIMES_H
#define PINE_UTIL_PRIMES_H

#include <core/defines.h>

namespace pine {

static constexpr int PrimeTableSize = 1000;
extern const int Primes[PrimeTableSize];
extern const int PrimeSums[PrimeTableSize];

}  // namespace pine

#endif  // PINE_UTIL_PRIMES_H