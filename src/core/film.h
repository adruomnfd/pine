#ifndef PINE_CORE_FILM_H
#define PINE_CORE_FILM_H

#include <core/spectrum.h>
#include <core/filter.h>
#include <util/parallel.h>

#include <memory>
#include <atomic>
#include <mutex>

namespace pine {

struct Pixel {
    AtomicFloat rgba[4];
    AtomicFloat weight;
};

struct Film {
    static Film Create(const Parameters& params);
    Film() = default;
    Film(vec2i size, Filter filter);

    void AddSample(vec2 pFilm, const Spectrum& sL) {
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
                pixel.rgba[0].Add(L[0] * weight);
                pixel.rgba[1].Add(L[1] * weight);
                pixel.rgba[2].Add(L[2] * weight);
                pixel.rgba[3].Add(weight);
                pixel.weight.Add(weight);
            }
    }

    Pixel& GetPixel(vec2i p) {
        return pixels[(size.y - 1 - p.y) * size.x + p.x];
    }

    vec2i Size() const {
        return size;
    }
    void Clear() {
        for (int i = 0; i < size.x * size.y; i++) {
            pixels[i].rgba[0] = 0.0f;
            pixels[i].rgba[1] = 0.0f;
            pixels[i].rgba[2] = 0.0f;
            pixels[i].rgba[3] = 0.0f;
            pixels[i].weight = 0.0f;
            rgba[i] = {};
        }
    }
    void Finalize() {
        CopyToRGBArray();
        ApplyToneMapping();
        ApplyGammaCorrection();
    }
    void WriteToDisk(std::string filename) const;

  private:
    float GetFilterValue(vec2 p) {
        vec2i pi = filterTableWidth * Min(Abs(p) / filter.Radius(), vec2(OneMinusEpsilon));
        return filterTable[pi.y * filterTableWidth + pi.x];
    }
    void CopyToRGBArray();
    void ApplyToneMapping();
    void ApplyGammaCorrection();

    vec2i size;
    Filter filter;
    std::shared_ptr<Pixel[]> pixels;
    std::shared_ptr<vec4[]> rgba;

    static constexpr int filterTableWidth = 16;
    float filterTable[filterTableWidth * filterTableWidth];
};

}  // namespace pine

#endif  // PINE_CORE_FILM_H