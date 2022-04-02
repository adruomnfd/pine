#ifndef PINE_CORE_CAMERA_H
#define PINE_CORE_CAMERA_H

#include <core/geometry.h>
#include <core/medium.h>
#include <core/film.h>
#include <util/taggedvariant.h>
#include <util/profiler.h>

namespace pine {

struct CameraSample {
    vec2 pFilm;
    Spectrum we;
    vec3 p;
    vec3 wo;
    float pdf = 1.0f;
};

struct ThinLenCamera {
    static ThinLenCamera Create(const Parameters& params);

    ThinLenCamera(Film film, vec3 from, vec3 to, float fov, float lensRadius, float focalDistance)
        : c2w(LookAt(from, to)),
          w2c(Inverse(c2w)),
          film(film),
          nLens(Normalize(to - from)),
          fov(std::tan(fov / 2)),
          fov2d(fov * film.Aspect(), fov),
          lensRadius(lensRadius),
          focalDistance(focalDistance),
          lensArea(lensRadius ? Pi * Sqr(lensRadius) : 1.0f),
          area(Sqr(2 * this->fov) * film.Aspect()){};

    Ray GenRay(vec2 pFilm, vec2 u2) const;
    Spectrum We(const Ray& ray, vec2& pFilm) const;
    SpatialPdf PdfWe(const Ray& ray) const;
    CameraSample SampleWi(vec3 p, vec2 u) const;
    Film& GetFilm() {
        return film;
    }

    mat4 c2w;
    mat4 w2c;
    Film film;
    vec3 nLens;
    float fov;
    vec2 fov2d;
    float lensRadius;
    float focalDistance;
    float lensArea;
    float area;
};

struct Camera : public TaggedVariant<ThinLenCamera> {
    using TaggedVariant::TaggedVariant;
    static Camera Create(const Parameters& params, Scene* scene);

    Ray GenRay(vec2 pFilm, vec2 u2) const {
        SampledProfiler _(ProfilePhase::GenerateRay);
        Ray ray = Dispatch([&](auto&& x) { return x.GenRay(pFilm, u2); });
        ray.medium = medium.get();
        return ray;
    }
    Spectrum We(const Ray& ray, vec2& pFilm) const {
        return Dispatch([&](auto&& x) { return x.We(ray, pFilm); });
    }
    SpatialPdf PdfWe(const Ray& ray) const {
        return Dispatch([&](auto&& x) { return x.PdfWe(ray); });
    }
    CameraSample SampleWi(vec3 p, vec2 u) const {
        return Dispatch([&](auto&& x) { return x.SampleWi(p, u); });
    }
    Film& GetFilm() {
        return Dispatch([&](auto&& x) -> auto& { return x.GetFilm(); });
    }

    std::shared_ptr<Medium> medium;
};

}  // namespace pine

#endif  // PINE_CORE_CAMERA_H