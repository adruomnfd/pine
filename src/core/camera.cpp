#include <core/camera.h>
#include <core/scene.h>
#include <util/parameters.h>
#include <util/misc.h>

namespace pine {

ThinLenCamera ThinLenCamera::Create(const Parameters& params) {
    return ThinLenCamera(CreateFilm(params["film"]), params.GetVec3("from"), params.GetVec3("to"),
                         params.GetFloat("fov"), params.GetFloat("lensRadius", 0.0f),
                         params.GetFloat("focusDistance", 1.0f));
}

static vec2 TransformToFilmSpace(vec2 up, vec2 fov2d) {
    up = (up - vec2(0.5f)) * 2;
    return up * fov2d;
}

static vec2 TransformFromFilmSpace(vec2 pFilm, vec2 fov2d) {
    vec2 up = pFilm / fov2d;
    return up / 2.0f + vec2(0.5f);
}

static vec2 ProjectZPlane(vec3 p, const mat4& m) {
    vec4 p4 = m * vec4(p, 1.0f);
    p4 /= p4.w;
    p4 /= p4.z;
    return p4;
}

Ray ThinLenCamera::GenRay(vec2 up, vec2 u2) const {
    vec2 pFilm = TransformToFilmSpace(up, fov2d);
    vec3 dir = Normalize(vec3(pFilm, 1.0f));
    vec3 pFocus = focalDistance * dir / dir.z;
    vec3 pLen = lensRadius * vec3(SampleDiskPolar(u2), 0.0f);
    Ray ray;
    ray.o = c2w * vec4(pLen, 1.0f);
    ray.d = (mat3)c2w * Normalize(pFocus - pLen);

    return ray;
}

Spectrum ThinLenCamera::We(const Ray& ray, vec2& pFilm) const {
    float cosTheta = Dot(ray.d, nLens);
    if (cosTheta <= 0.0f)
        return 0.0f;

    vec3 pFocus = ProjectZPlane(ray(focalDistance / cosTheta), w2c);
    pFilm = TransformFromFilmSpace(pFocus, fov2d);
    if (!Inside(pFilm, vec2(0.0f), vec2(1.0f)))
        return 0.0f;
    float cos4Theta = pstd::sqr(pstd::sqr(cosTheta));
    return Spectrum(1.0f / (area * lensArea * cos4Theta));
}

SpatialPdf ThinLenCamera::PdfWe(const Ray& ray) const {
    float cosTheta = Dot(ray.d, nLens);
    if (cosTheta <= 0.0f)
        return {};

    vec3 pFocus = ProjectZPlane(ray(focalDistance / cosTheta), w2c);
    vec2 pFilm = TransformFromFilmSpace(pFocus, fov2d);
    if (!Inside(pFilm, vec2(0.0f), vec2(1.0f)))
        return {};

    SpatialPdf pdf;
    pdf.pos = 1.0f / lensArea;
    pdf.dir = 1.0f / (area * cosTheta * cosTheta * cosTheta);
    return pdf;
}

CameraSample ThinLenCamera::SampleWi(vec3 p, vec2 u) const {
    CameraSample cs;
    vec2 pLens = lensRadius * SampleDiskConcentric(u);
    vec3 pLensWorld = c2w * vec4(pLens, 0.0f, 1.0f);

    float dist;
    cs.p = pLensWorld;
    cs.wo = Normalize(pLensWorld - p, dist);
    cs.pdf = pstd::sqr(dist) / (AbsDot(nLens, -cs.wo) * lensArea);
    cs.we = We(Ray(pLensWorld, -cs.wo), cs.pFilm);

    return cs;
}

Camera CreateCamera(const Parameters& params, Scene* scene) {
    Camera camera;
    pstd::string type = params.GetString("type");

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
        LOG("[Camera]In medium");
    return camera;
}

}  // namespace pine