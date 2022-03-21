#ifndef PINE_CORE_SAMPLER_H
#define PINE_CORE_SAMPLER_H

#include <core/vecmath.h>
#include <util/taggedvariant.h>
#include <util/profiler.h>
#include <util/rng.h>

#include <vector>

namespace pine {

struct UniformSampler {
    static UniformSampler Create(const Parameters& params);
    UniformSampler(int samplesPerPixel, int seed = 0) : samplesPerPixel(samplesPerPixel), rng(seed) {
    }

    int SamplesPerPixel() const {
        return samplesPerPixel;
    }
    void StartPixel(const vec2i&, int) {
    }
    void StartNextSample() {
    }
    float Get1D() {
        return rng.Uniformf();
    }
    vec2 Get2D() {
        return rng.Uniform2f();
    }

    int samplesPerPixel;
    RNG rng;
};

struct StratifiedSampler {
    static StratifiedSampler Create(const Parameters& params);
    StratifiedSampler(int xPixelSamples, int yPixelSamples, bool jitter)
        : xPixelSamples(xPixelSamples), yPixelSamples(yPixelSamples), jitter(jitter) {
        samplesPerPixel = xPixelSamples * yPixelSamples;
    }

    int SamplesPerPixel() const {
        return samplesPerPixel;
    }
    void StartPixel(vec2i p, int index) {
        pixel = p;
        sampleIndex = index;
        dimension = 0;
    }
    void StartNextSample() {
        sampleIndex++;
        dimension = 0;
    }
    float Get1D() {
        int stratum = (sampleIndex + Hash(pixel, dimension)) % samplesPerPixel;
        dimension += 1;

        float delta = jitter ? rng.Uniformf() : 0.5f;

        return (stratum + delta) / SamplesPerPixel();
    }
    vec2 Get2D() {
        int stratum = (sampleIndex + Hash(pixel, dimension)) % samplesPerPixel;
        dimension += 2;

        int x = stratum % xPixelSamples, y = stratum / xPixelSamples;
        float dx = jitter ? rng.Uniformf() : 0.5f, dy = jitter ? rng.Uniformf() : 0.5f;

        return {(x + dx) / xPixelSamples, (y + dy) / yPixelSamples};
    }

    int xPixelSamples, yPixelSamples;
    int samplesPerPixel;
    RNG rng;
    vec2i pixel;
    int sampleIndex;
    int dimension;
    bool jitter;
};

struct HaltonSampler {
    enum class RandomizeStrategy { None, PermuteDigits };

    static HaltonSampler Create(const Parameters& params);
    HaltonSampler(int samplesPerPixel, vec2i filmSize, RandomizeStrategy randomizeStrategy);

    int SamplesPerPixel() const {
        return samplesPerPixel;
    }
    void StartPixel(vec2i p, int sampleIndex);
    void StartNextSample() {
        haltonIndex += sampleStride;
        dimension = 2;
    }
    float Get1D();
    vec2 Get2D();
    float SampleDimension(int dimension) const;
    const uint16_t* PermutationForDimension(int dim) const;

    int samplesPerPixel = 0;
    static constexpr int MaxHaltonResolution = 128;
    vec2i baseScales, baseExponents;
    int multInverse[2] = {};

    int sampleStride = 0;
    int64_t haltonIndex = 0;
    int dimension = 0;
    RandomizeStrategy randomizeStrategy;

    static inline std::vector<uint16_t> radicalInversePermutations;
};

struct ZeroTwoSequenceSampler {
    static ZeroTwoSequenceSampler Create(const Parameters& params);
    ZeroTwoSequenceSampler(int samplesPerPixel, int nSampledDimensions);

    int SamplesPerPixel() const {
        return samplesPerPixel;
    }
    void StartPixel(vec2i p, int sampleIndex);
    void StartNextSample() {
        currentSampleIndex++;
        current1DDimension = 0;
        current2DDimension = 0;
    }
    float Get1D();
    vec2 Get2D();

    int samplesPerPixel;
    int nSampledDimensions;

    int currentSampleIndex = 0;
    int current1DDimension = 0, current2DDimension = 0;
    std::vector<std::vector<float>> samples1D;
    std::vector<std::vector<vec2>> samples2D;
    RNG rng;
};

struct SobolSampler {
    enum class RandomizeStrategy { None, BinaryPermutate, FastOwen, Owen };

    static SobolSampler Create(const Parameters& params);
    SobolSampler(int samplesPerPixel, vec2i filmSize, RandomizeStrategy randomizeStrategy);

    int SamplesPerPixel() const {
        return samplesPerPixel;
    }
    void StartPixel(vec2i p, int sampleIndex);
    void StartNextSample();
    float Get1D();
    vec2 Get2D();

    float SampleDimension(int dim) const;

    int samplesPerPixel = 0;
    RandomizeStrategy randomizeStrategy;
    int dimension = 0;
    int scale = 0, log2Scale = 0;
    vec2 pixel;
    int sampleIndex = 0;
    int64_t sobolIndex = 0;
};

struct MltSampler {
    MltSampler(float sigma, float largeStepProbability, int seed)
        : rng(seed), sigma(sigma), largeStepProbability(largeStepProbability){};
    int SamplesPerPixel() const {
        LOG_FATAL("[MltSampler]SamplesPerPixel() is not implemented");
        return 0;
    }
    void StartPixel(vec2i, int) {
        LOG_FATAL("[MltSampler]StartPixel() is not implemented");
    }
    void StartNextSample() {
        sampleIndex++;
        dimension = 0;
        largeStep = rng.Uniformf() < largeStepProbability;
    }
    float Get1D() {
        EnsureReady(dimension);
        return X[dimension++].value;
    }
    vec2 Get2D() {
        return {Get1D(), Get1D()};
    }

    void Accept() {
        if (largeStep)
            lastLargeStepIndex = sampleIndex;
    }
    void Reject() {
        for (auto& Xi : X)
            if (Xi.lastModificationIndex == sampleIndex)
                Xi.Restore();
        --sampleIndex;
    }

  private:
    void EnsureReady(int dim);

    struct PrimarySample {
        void Backup() {
            valueBackup = value;
            modifyBackup = lastModificationIndex;
        }
        void Restore() {
            value = valueBackup;
            lastModificationIndex = modifyBackup;
        }

        float value = 0, valueBackup = 0;
        int64_t lastModificationIndex = 0;
        int64_t modifyBackup = 0;
    };

    RNG rng;
    const float sigma, largeStepProbability;
    std::vector<PrimarySample> X;
    int64_t sampleIndex = 0;
    int64_t dimension = 0;
    bool largeStep = true;
    int64_t lastLargeStepIndex = 0;
};

struct Sampler : TaggedVariant<UniformSampler, StratifiedSampler, HaltonSampler,
                               ZeroTwoSequenceSampler, SobolSampler, MltSampler> {
    using TaggedVariant::TaggedVariant;
    static Sampler Create(const Parameters& params);

    int SamplesPerPixel() const {
        SampledProfiler _(ProfilePhase::GenerateSamples);
        return Dispatch([&](auto&& x) { return x.SamplesPerPixel(); });
    }
    void StartPixel(vec2i p, int sampleIndex) {
        SampledProfiler _(ProfilePhase::GenerateSamples);
        return Dispatch([&](auto&& x) { return x.StartPixel(p, sampleIndex); });
    }
    void StartNextSample() {
        SampledProfiler _(ProfilePhase::GenerateSamples);
        return Dispatch([&](auto&& x) { return x.StartNextSample(); });
    }
    float Get1D() {
        SampledProfiler _(ProfilePhase::GenerateSamples);
        return Dispatch([&](auto&& x) { return x.Get1D(); });
    }
    vec2 Get2D() {
        SampledProfiler _(ProfilePhase::GenerateSamples);
        return Dispatch([&](auto&& x) { return x.Get2D(); });
    }
    Sampler Clone() const {
        return Dispatch([&](auto&& x) { return Sampler(x); });
    }
};

}  // namespace pine

#endif  // PINE_CORE_SAMPLER_H