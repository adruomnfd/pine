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
    AtomicFloat splatXYZ[3];
    AtomicFloat weight;
};

struct Film {
    Film() = default;
    Film(vec2i size, Filter filter, std::string outputFileName, bool applyToneMapping,
         bool reportAverageColor);

    void AddSample(vec2 pFilm, const Spectrum& sL) {
        SampledProfiler _(ProfilePhase::FilmAddSample);
        pFilm *= size;
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
            }
    }
    void AddSplat(vec2 pFilm, const Spectrum& sL) {
        SampledProfiler _(ProfilePhase::FilmAddSample);
        vec2i p = pFilm * size;
        if (!Inside(p, vec2i(0), size))
            return;
        float xyz[3];
        sL.ToXYZ(xyz);

        auto& pixel = GetPixel(p);
        pixel.splatXYZ[0].Add(xyz[0]);
        pixel.splatXYZ[1].Add(xyz[1]);
        pixel.splatXYZ[2].Add(xyz[2]);
    }

    Pixel& GetPixel(vec2i p) {
        return pixels[(size.y - 1 - p.y) * size.x + p.x];
    }

    vec2i Size() const {
        return size;
    }
    float Aspect() const {
        return (float)size.x / size.y;
    }
    void Clear();
    void Finalize(float splatMultiplier = 1.0f);
    void WriteToDisk(std::string filename) const;

  private:
    float GetFilterValue(vec2 p) {
        vec2i pi = filterTableWidth * Min(Abs(p) / filter.Radius(), vec2(OneMinusEpsilon));
        return filterTable[pi.y * filterTableWidth + pi.x];
    }
    void CopyToRGBArray(float splatMultiplier);
    void ApplyToneMapping();
    void ApplyGammaCorrection();

    vec2i size;
    Filter filter;
    std::shared_ptr<Pixel[]> pixels;
    std::shared_ptr<vec4[]> rgba;

    static constexpr int filterTableWidth = 16;
    float filterTable[filterTableWidth * filterTableWidth];

    std::string outputFileName;
    bool applyToneMapping = true;
    bool reportAverageColor = false;
    int frameId = 0;
};

Film CreateFilm(const Parameters& params);

}  // namespace pine

#endif  // PINE_CORE_FILM_H