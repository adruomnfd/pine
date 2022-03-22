#ifndef PINE_CORE_FILM_H
#define PINE_CORE_FILM_H

#include <core/spectrum.h>
#include <core/filter.h>
#include <util/parallel.h>
#include <util/profiler.h>

#include <memory>
#include <atomic>
#include <mutex>

namespace pine {

struct Pixel {
    AtomicFloat rgb[3];
    AtomicFloat weight;
    std::atomic<int> nsamples{0};
};

struct Film {
    static Film Create(const Parameters& params);
    Film() = default;
    Film(vec2i size, Filter filter, std::string outputFileName);

    void AddSample(vec2 pFilm, const Spectrum& sL) {
        SampledProfiler _(ProfilePhase::FilmAddSample);
        pFilm -= vec2(0.5f);
        vec2i p0 = Ceil(pFilm - filter.Radius());
        vec2i p1 = Floor(pFilm + filter.Radius());
        p0 = Max(p0, vec2i(0));
        p1 = Min(p1, size - vec2i(1));
        vec3 L = sL.ToRGB();

        for (int y = p0.y; y <= p1.y; y++)
            for (int x = p0.x; x <= p1.x; x++) {
                float weight = GetFilterValue(vec2(x, y) - pFilm);
                Pixel& pixel = GetPixel(vec2i(x, y));
                pixel.rgb[0].Add(L[0] * weight);
                pixel.rgb[1].Add(L[1] * weight);
                pixel.rgb[2].Add(L[2] * weight);
                pixel.weight.Add(weight);
                ++pixel.nsamples;
            }
    }

    Pixel& GetPixel(vec2i p) {
        return pixels[(size.y - 1 - p.y) * size.x + p.x];
    }

    vec2i Size() const {
        return size;
    }
    void Clear() {
        for (int i = 0; i < Area(size); i++) {
            pixels[i].rgb[0] = 0.0f;
            pixels[i].rgb[1] = 0.0f;
            pixels[i].rgb[2] = 0.0f;
            pixels[i].weight = 0.0f;
            rgba[i] = {};
        }
    }
    void Finalize(float multiplier = 1.0f, bool cumulative = false) {
        CopyToRGBArray(multiplier, cumulative);
        ApplyToneMapping();
        ApplyGammaCorrection();
        WriteToDisk(outputFileName);
    }
    void WriteToDisk(std::string filename) const;

  private:
    float GetFilterValue(vec2 p) {
        vec2i pi = filterTableWidth * Min(Abs(p) / filter.Radius(), vec2(OneMinusEpsilon));
        return filterTable[pi.y * filterTableWidth + pi.x];
    }
    void CopyToRGBArray(float multiplier, bool cumulative);
    void ApplyToneMapping();
    void ApplyGammaCorrection();

    vec2i size;
    Filter filter;
    std::shared_ptr<Pixel[]> pixels;
    std::shared_ptr<vec4[]> rgba;

    static constexpr int filterTableWidth = 16;
    float filterTable[filterTableWidth * filterTableWidth];

    std::string outputFileName;
};

}  // namespace pine

#endif  // PINE_CORE_FILM_H