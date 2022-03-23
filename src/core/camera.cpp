#include <core/camera.h>
#include <core/scene.h>
#include <util/parameters.h>
#include <util/misc.h>

namespace pine {

ThinLenCamera ThinLenCamera::Create(const Parameters& params) {
    return ThinLenCamera(Film::Create(params["film"]), params.GetVec3("from"), params.GetVec3("to"),
                         params.GetFloat("fov"), params.GetFloat("lensRadius", 0.0f),
                         params.GetFloat("focusDistance", 1.0f));
}

Ray ThinLenCamera::GenRay(vec2 pFilm, vec2 u2) const {
    pFilm = vec2(film.Aspect(), 1.0f) * (pFilm - vec2(0.5f));
    vec3 dir = Normalize(vec3(pFilm * fov, 1.0f));
    vec3 pFocus = focalDistance * dir / dir.z;
    vec3 pLen = lensRadius * vec3(SampleDiskPolar(u2), 0.0f);
    Ray ray;
    ray.o = c2w * vec4(pLen, 1.0f);
    ray.d = (mat3)c2w * Normalize(pFocus - pLen);

    return ray;
}

Spectrum ThinLenCamera::We(const Ray& ray, vec2& pFilm) const {
    float cosTheta = Dot(ray.d, (mat3)c2w * vec3(0, 0, 1));
    if (cosTheta <= 0.0f)
        return 0.0f;

    vec3 pFocus = w2c * vec4(ray(focalDistance / cosTheta), 1.0f);
    pFilm = pFocus / pFocus.z / fov;
    pFilm = pFilm / vec2(film.Aspect(), 1.0f) + vec2(0.5f);
    if (!Inside(pFilm, vec2(0.0f), vec2(1.0f)))
        return 0.0f;

    float A = 1.0f;
    float lensArea = lensRadius != 0.0f ? Pi * lensRadius * lensRadius : 1.0f;
    float cos4Theta = Sqr(Sqr(cosTheta));

    return Spectrum(1.0f / (A * lensArea * cos4Theta));
}
SpatialPdf ThinLenCamera::PdfWe(const Ray& ray) const {
    float cosTheta = Dot(ray.d, (mat3)c2w * vec3(0, 0, 1));
    if (cosTheta <= 0.0f) {
        return {};
    }

    vec3 pFocus = w2c * vec4(ray(focalDistance / cosTheta), 1.0f);
    vec2 pFilm = pFocus / pFocus.z / fov;
    pFilm = pFilm / vec2(film.Aspect(), 1.0f) + vec2(0.5f);
    if (!Inside(pFilm, vec2(0.0f), vec2(1.0f))) {
        return {};
    }

    float A = Area(film.Size());
    float lensArea = lensRadius != 0.0f ? Pi * lensRadius * lensRadius : 1.0f;

    SpatialPdf pdf;
    pdf.pos = 1.0f / lensArea;
    pdf.dir = 1.0f / (A * cosTheta * cosTheta * cosTheta);
    return pdf;
}
CameraSample ThinLenCamera::SampleWi(vec3 p, vec2 u) const {
    CameraSample cs;
    vec2 pLens = lensRadius * SampleDiskConcentric(u);
    vec3 pLensWorld = c2w * vec4(pLens, 0.0f, 1.0f);
    vec3 nLens = (mat3)c2w * vec3(0, 0, 1);

    float lensArea = lensRadius != 0.0f ? Pi * lensRadius * lensRadius : 1.0f;

    float dist;
    cs.p = pLensWorld;
    cs.wo = Normalize(pLensWorld - p, dist);
    cs.pdf = Sqr(dist) / (AbsDot(nLens, cs.wo) * lensArea);
    cs.we = We(Ray(pLensWorld, -cs.wo), cs.pFilm);

    return cs;
}

Camera Camera::Create(const Parameters& params, Scene* scene) {
    Camera camera;
    std::string type = params.GetString("type");

    SWITCH(type) {
        CASE("ThinLen") camera = ThinLenCamera(ThinLenCamera::Create(params));
        DEFAULT {
            LOG_WARNING("[Camera][Create]Unknown type \"&\"", type);
            camera = ThinLenCamera(ThinLenCamera::Create(params));
        }
    }

    if (auto name = params.TryGetString("medium")) {
        auto medium = Find(scene->mediums, *name);
        if (!medium)
            LOG_WARNING("[Camera][Create]Medium \"&\" is not found", name);
        camera.medium = *medium;
    }
    if (camera.medium)
        LOG_VERBOSE("[Camera]In medium");
    return camera;
}

}  // namespace pine