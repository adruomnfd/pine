#ifndef PINE_CORE_SAMPLER_H
#define PINE_CORE_SAMPLER_H

#include <core/vecmath.h>
#include <util/rng.h>
#include <util/taggedptr.h>

#include <vector>

namespace pine {

struct UniformSampler {
    static UniformSampler Create(const Parameters& params);
    UniformSampler(int samplesPerPixel) : samplesPerPixel(samplesPerPixel) {
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
        dimension = 2;
        haltonIndex += sampleStride;
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

struct Sampler : TaggedPointer<UniformSampler, StratifiedSampler, HaltonSampler,
                               ZeroTwoSequenceSampler, SobolSampler> {
    using TaggedPointer::TaggedPointer;

    static Sampler Create(const Parameters& params);
    static void Destory(Sampler sampler) {
        sampler.Delete();
    }

    int SamplesPerPixel() const {
        return Dispatch([](auto ptr) { return ptr->SamplesPerPixel(); });
    }
    void StartPixel(vec2i p, int sampleIndex) {
        return Dispatch([&](auto ptr) { return ptr->StartPixel(p, sampleIndex); });
    }
    void StartNextSample() {
        return Dispatch([](auto ptr) { return ptr->StartNextSample(); });
    }
    float Get1D() {
        return Dispatch([](auto ptr) { return ptr->Get1D(); });
    }
    vec2 Get2D() {
        return Dispatch([](auto ptr) { return ptr->Get2D(); });
    }
    Sampler Clone() const {
        return Dispatch([](auto ptr) {
            using T = std::decay_t<decltype(*ptr)>;
            return Sampler(new T(*ptr));
        });
    }
};

}  // namespace pine

#endif  // PINE_CORE_SAMPLER_H