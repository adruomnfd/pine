#include <core/lowdiscrepancy.h>

namespace pine {

std::vector<uint16_t> ComputeRadicalInversePermutations(RNG& rng) {
    std::vector<uint16_t> perms;
    int permArraySize = 0;
    for (int i = 0; i < PrimeTableSize; i++)
        permArraySize += Primes[i];
    perms.resize(permArraySize);

    uint16_t* p = &perms[0];
    for (int i = 0; i < PrimeTableSize; i++) {
        for (int j = 0; j < Primes[i]; j++)
            p[j] = j;
        Shuffle(p, Primes[i], 1, rng);
        p += Primes[i];
    }

    return perms;
}

}  // namespace pine