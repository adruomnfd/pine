#include <core/sampler.h>
#include <core/lowdiscrepancy.h>
#include <util/parameters.h>
#include <util/primes.h>
#include <util/sobolmetrices.h>

namespace pine {

static void extendedGCD(uint64_t a, uint64_t b, int64_t& gcd, int64_t& x, int64_t& y) {
    int d = a / b;
    int r = a - d * b;

    if (r == 0) {
        gcd = b;
        x = 0;
        y = 1;
        return;
    }

    int64_t nx, ny;
    extendedGCD(b, r, gcd, nx, ny);
    x = ny;
    y = nx - d * ny;
}

static uint64_t multiplicativeInverse(int64_t a, int64_t n) {
    int64_t gcd, x, y;
    extendedGCD(a, n, gcd, x, y);
    return Mod(x, n);
}

HaltonSampler::HaltonSampler(int samplesPerPixel, vec2i filmSize,
                             RandomizeStrategy randomizeStrategy)
    : samplesPerPixel(samplesPerPixel), randomizeStrategy(randomizeStrategy) {
    if (randomizeStrategy == RandomizeStrategy::PermuteDigits &&
        radicalInversePermutations.size() == 0) {
        RNG rng;
        radicalInversePermutations = ComputeRadicalInversePermutations(rng);
    }

    for (int i = 0; i < 2; i++) {
        int base = (i == 0) ? 2 : 3;
        int scale = 1, exp = 0;
        while (scale < min(filmSize[i], MaxHaltonResolution)) {
            scale *= base;
            ++exp;
        }
        baseScales[i] = scale;
        baseExponents[i] = exp;
    }

    multInverse[0] = multiplicativeInverse(baseScales[0], baseScales[1]);
    multInverse[1] = multiplicativeInverse(baseScales[1], baseScales[0]);

    sampleStride = baseScales[0] * baseScales[1];
}

void HaltonSampler::StartPixel(vec2i p, int sampleIndex) {
    haltonIndex = 0;
    if (sampleStride > 1) {
        vec2i pm = {Mod(p[0], MaxHaltonResolution), Mod(p[1], MaxHaltonResolution)};
        for (int i = 0; i < 2; i++) {
            uint64_t dimOffset = InverseRadicalInverse(pm[i], i == 0 ? 2 : 3, baseExponents[i]);
            haltonIndex += dimOffset * baseScales[1 - i] * multInverse[1 - i];
        }
        haltonIndex %= sampleStride;
    }

    haltonIndex += sampleIndex * sampleStride;
    dimension = 2;
}
float HaltonSampler::Get1D() {
    if (dimension >= PrimeTableSize)
        dimension = 2;
    return SampleDimension(dimension++);
}
vec2 HaltonSampler::Get2D() {
    if (dimension + 1 >= PrimeTableSize)
        dimension = 2;
    int dim = dimension;
    dimension += 2;
    return {SampleDimension(dim), SampleDimension(dim + 1)};
}
float HaltonSampler::SampleDimension(int dim) const {
    switch (randomizeStrategy) {
    case RandomizeStrategy::None: return RadicalInverse(dim, haltonIndex);
    case RandomizeStrategy::PermuteDigits:
        return ScrambledRadicalInverse(dim, haltonIndex, PermutationForDimension(dim));
    default: return RadicalInverse(dim, haltonIndex);
    }
}

const uint16_t* HaltonSampler::PermutationForDimension(int dim) const {
    CHECK_LT(dim, PrimeTableSize);
    return &radicalInversePermutations[PrimeSums[dim]];
}

ZeroTwoSequenceSampler::ZeroTwoSequenceSampler(int spp, int nSampledDimensions)
    : nSampledDimensions(nSampledDimensions) {
    samplesPerPixel = RoundUpPow2(spp);
    for (int i = 0; i < nSampledDimensions; i++) {
        samples1D.push_back(std::vector<float>(samplesPerPixel));
        samples2D.push_back(std::vector<vec2>(samplesPerPixel));
    }
}
void ZeroTwoSequenceSampler::StartPixel(vec2i, int sampleIndex) {
    currentSampleIndex = sampleIndex;
    current1DDimension = current2DDimension = 0;
    if (sampleIndex == 0) {
        for (auto& s : samples1D)
            VanDerCorput(samplesPerPixel, &s[0], rng);
        for (auto& s : samples2D)
            Sobol2D(samplesPerPixel, &s[0], rng);
    }
}
float ZeroTwoSequenceSampler::Get1D() {
    if (current1DDimension < (int)samples1D.size())
        return samples1D[current1DDimension++][currentSampleIndex];
    else
        return rng.Uniformf();
}
vec2 ZeroTwoSequenceSampler::Get2D() {
    if (current2DDimension < (int)samples2D.size())
        return samples2D[current2DDimension++][currentSampleIndex];
    else
        return rng.Uniform2f();
}

SobolSampler::SobolSampler(int spp, vec2i filmSize, RandomizeStrategy randomizeStrategy)
    : randomizeStrategy(randomizeStrategy) {
    samplesPerPixel = spp;
    scale = RoundUpPow2(max(filmSize.x, filmSize.y));
    log2Scale = Log2Int(scale);
}

void SobolSampler::StartPixel(vec2i p, int index) {
    pixel = p;
    dimension = 2;
    sampleIndex = index;
    sobolIndex = SobolIntervalToIndex(log2Scale, sampleIndex, pixel);
}
void SobolSampler::StartNextSample() {
    dimension = 2;
    sampleIndex++;
    sobolIndex = SobolIntervalToIndex(log2Scale, sampleIndex, pixel);
}
float SobolSampler::Get1D() {
    if (dimension >= NSobolDimensions)
        dimension = 2;
    return SampleDimension(dimension++);
}
vec2 SobolSampler::Get2D() {
    if (dimension >= NSobolDimensions)
        dimension = 2;
    int dim = dimension;
    dimension += 2;
    return {SampleDimension(dim), SampleDimension(dim + 1)};
}

struct NoRandomizer {
    uint32_t operator()(uint32_t v) const {
        return v;
    }
};

struct BinaryPermuteScrambler {
    BinaryPermuteScrambler(uint32_t permutation) : permutation(permutation) {
    }
    uint32_t operator()(uint32_t v) const {
        return permutation ^ v;
    }
    uint32_t permutation;
};

struct FastOwenScramber {
    FastOwenScramber(uint32_t seed) : seed(seed) {
    }

    uint32_t operator()(uint32_t v) const {
        v = ReverseBits32(v);
        v ^= v * 0x3d20adea;
        v += seed;
        v *= (seed >> 16) | 1;
        v ^= v * 0x05526c56;
        v ^= v * 0x53a22864;
        return ReverseBits32(v);
    }

    uint32_t seed;
};

struct OwenScramber {
    OwenScramber(uint32_t seed) : seed(seed) {
    }

    uint32_t operator()(uint32_t v) const {
        if (seed & 1)
            v ^= 1u << 31;
        for (int b = 1; b < 32; b++) {
            uint32_t mask = (~0u) << (32 - b);
            if ((uint32_t)MixBits((v & mask) ^ seed) & (1u << b))
                v ^= 1u << (31 - b);
        }
        return v;
    }

    uint32_t seed;
};

float SobolSampler::SampleDimension(int dim) const {
    uint32_t hash = Hash(dim);
    switch (randomizeStrategy) {
    case RandomizeStrategy::None: return SobolSample(sobolIndex, dim, NoRandomizer());
    case RandomizeStrategy::BinaryPermutate:
        return SobolSample(sobolIndex, dim, BinaryPermuteScrambler(hash));
    case RandomizeStrategy::Owen: return SobolSample(sobolIndex, dim, OwenScramber(hash));
    case RandomizeStrategy::FastOwen: return SobolSample(sobolIndex, dim, FastOwenScramber(hash));
    default: return SobolSample(sobolIndex, dim, NoRandomizer());
    }
}

UniformSampler UniformSampler::Create(const Parameters& params) {
    return UniformSampler(params.GetInt("samplesPerPixel"));
}

StratifiedSampler StratifiedSampler::Create(const Parameters& params) {
    return StratifiedSampler(params.GetInt("xPixelSamples"), params.GetInt("yPixelSamples"),
                             params.GetBool("jitter", true));
}

HaltonSampler HaltonSampler::Create(const Parameters& params) {
    RandomizeStrategy randomizeStrategy = RandomizeStrategy::PermuteDigits;
    SWITCH(params.GetString("randomizeStrategy")) {
        CASE("none") randomizeStrategy = RandomizeStrategy::None;
        CASE("permutateDigits") randomizeStrategy = RandomizeStrategy::PermuteDigits;
    }
    return HaltonSampler(params.GetInt("samplesPerPixel"), params.GetVec2i("filmSize"),
                         randomizeStrategy);
}

ZeroTwoSequenceSampler ZeroTwoSequenceSampler::Create(const Parameters& params) {
    return ZeroTwoSequenceSampler(params.GetInt("samplesPerPixel"),
                                  params.GetInt("nSampledDimensions"));
}

SobolSampler SobolSampler::Create(const Parameters& params) {
    RandomizeStrategy randomizeStrategy = RandomizeStrategy::BinaryPermutate;
    SWITCH(params.GetString("randomizeStrategy")) {
        CASE("none") randomizeStrategy = RandomizeStrategy::None;
        CASE("binaryPermutate") randomizeStrategy = RandomizeStrategy::BinaryPermutate;
        CASE("Owen") randomizeStrategy = RandomizeStrategy::Owen;
        CASE("fastOwen") randomizeStrategy = RandomizeStrategy::FastOwen;
    }
    return SobolSampler(params.GetInt("samplesPerPixel"), params.GetVec2i("filmSize"),
                        randomizeStrategy);
}

Sampler Sampler::Create(const Parameters& params) {
    std::string type = params.GetString("type");
    SWITCH(type) {
        CASE("Uniform") return new UniformSampler(UniformSampler::Create(params));
        CASE("Stratified") return new StratifiedSampler(StratifiedSampler::Create(params));
        CASE("Halton") return new HaltonSampler(HaltonSampler::Create(params));
        CASE("ZeroTwoSequence")
        return new ZeroTwoSequenceSampler(ZeroTwoSequenceSampler::Create(params));
        CASE("Sobol") return new SobolSampler(SobolSampler::Create(params));
        DEFAULT {
            LOG_WARNING("[Sampler][Create]Unknown type \"&\"", type);
            return new UniformSampler(UniformSampler::Create(params));
        }
    }
}

}  // namespace pine