#include <core/integrator.h>
#include <core/sampling.h>
#include <core/scene.h>
#include <core/color.h>
#include <util/parallel.h>
#include <util/profiler.h>
#include <util/fileio.h>
#include <util/rng.h>
#include <impl/integrators/ao.h>
#include <impl/integrators/path.h>
#include <impl/integrators/bvhviz.h>
#include <impl/integrators/photon.h>

namespace pine {

// Integrator definition
AOV Integrator::GetAOV(std::string name) {
    if (aovs.find(name) == aovs.end())
        LOG_WARNING("[Integrator]AOV \"&\" does not exist", name);
    return aovs[name];
}
std::shared_ptr<Integrator> Integrator::Create(const Parameters& parameters) {
    std::string type = parameters.GetString("type");
    SWITCH(type) {
        CASE("AO") return std::make_shared<AOIntegrator>(parameters);
        CASE("Path") return std::make_shared<PathIntegrator>(parameters);
        CASE("BvhViz") return std::make_shared<BvhVizIntegrator>(parameters);
        CASE("Photon") return std::make_shared<PhotonIntegrator>(parameters);
        DEFAULT {
            LOG_WARNING("[Integrator][Create]Unknown type \"&\"", type);
            return std::make_shared<AOIntegrator>(parameters);
        }
    }
}
Integrator::Integrator(const Parameters& parameters) {
    samplePerPixel = parameters.GetInt("samplePerPixel", 32);
    filmSize = parameters.GetVec2i("filmSize", vec2i(1280, 720));

    film = Image(filmSize);
    aovs["result"] = {film.Data(), filmSize};

    pr = ProgressReporter("Rendering", "Samples", samplePerPixel, filmSize.x * filmSize.y);
}
void Integrator::Initialize(const Scene* scene) {
    Profiler _("IntegratorInit");
    this->scene = scene;
    outputFileName = scene->parameters.GetString("outputFileName");
    sunDirection =
        Normalize(scene->parameters["atmosphere"].GetVec3("sunDirection", vec3(1, 4, 1)));
    sunIntensity = scene->parameters["atmosphere"].GetFloat("sunIntensity", 1.0f);
}

// RayIntegrator definition
RayIntegrator::RayIntegrator(const Parameters& parameters)
    : Integrator(parameters), parameters(parameters) {
}
void RayIntegrator::Initialize(const Scene* scene) {
    Integrator::Initialize(scene);
    for (const auto& mesh : scene->meshes) {
        std::shared_ptr<Accel> accel = Accel::Create(parameters["accel"]);
        accel->Initialize(&mesh);
        accels.push_back(accel);
    }
}
bool RayIntegrator::Hit(Ray ray) {
    for (const auto& accel : accels)
        if (accel->Hit(ray))
            return true;
    for (const auto& shape : scene->shapes)
        if (shape.Hit(ray))
            return true;
    return false;
}
bool RayIntegrator::Intersect(Ray& ray, Interaction& it) {
    bool hit = false;

    for (int i = 0; i < (int)accels.size(); i++)
        if (accels[i]->Intersect(ray, it)) {
            hit = true;
            it.material = scene->meshes[i].material;
        }

    for (const auto& shape : scene->shapes)
        if (shape.Intersect(ray, it)) {
            hit = true;
            it.material = shape.material;
        }

    return hit;
}
bool RayIntegrator::IntersectTr(Ray ray, vec3& tr, RNG& rng) {
    SampledProfiler _(ProfilePhase::IntersectTr);

    vec3 p2 = ray(ray.tmax);
    Interaction it;
    tr = vec3(1.0f);

    while (true) {
        bool hitSurface = Intersect(ray, it);
        if (ray.medium)
            tr *= ray.medium.Tr(ray, rng);
        if (!hitSurface)
            return false;
        if (it.material)
            return true;
        ray = it.SpawnRayTo(p2);
    }
}
vec3 RayIntegrator::EstimateDirect(Ray ray, Interaction it, RNG& rng) {
    SampledProfiler _(ProfilePhase::EstimateDirect);

    LightSample ls;

    float lightChoiceProb = scene->lights.size() ? (sunIntensity == 0.0f ? 1.0f : 0.5f) : 0.0f;

    if (rng.Uniformf() < lightChoiceProb) {
        uint64_t lightIndex = rng.Uniformf() * scene->lights.size();
        ls = scene->lights[lightIndex].Sample(it.p, rng.Uniformf(), rng.Uniform2f());
        ls.pdf = ls.pdf / lightChoiceProb;
    } else {
        ls.Le = AtmosphereColor(sunDirection, sunDirection, sunIntensity);
        ls.wo = sunDirection;
        ls.distance = 1e+10f;
        ls.p = ls.wo * ls.distance;
        ls.isDelta = true;
        ls.pdf = 1.0f / (1.0f - lightChoiceProb);
    }

    vec3 tr = vec3(1.0f);
    if (IntersectTr(it.SpawnRayTo(ls.p), tr, rng))
        return vec3(0.0f);

    if (it.IsSurfaceInteraction()) {
        MaterialEvalContext mc(it.p, it.n, it.uv, -ray.d, ls.wo);
        vec3 f = it.material.F(mc);
        float pdf = it.material.PDF(mc);
        return tr * f * AbsDot(ls.wo, it.n) * ls.Le * BalanceHeuristic(1, ls.pdf, 1, pdf) / ls.pdf;
    } else {
        return tr * it.phaseFunction.F(-ray.d, ls.wo) * ls.Le / ls.pdf;
    }
}

// PixelSampleIntegrator definition
void PixelSampleIntegrator::Render() {
    Profiler _("Render");
    sampleIndex = 0;

    while (NextIteration()) {
    }

    ParallelFor(filmSize, [&](vec2i p) {
        film[p] = vec4(Pow(Uncharted2Flimic(film[p]), 1.0f / 2.2f), 1.0f);
    });
    SaveBMPImage(outputFileName, film.Size(), 4, (float*)film.Data());
}
bool PixelSampleIntegrator::NextIteration() {
    if (sampleIndex >= samplePerPixel)
        return false;

    pr.Report(sampleIndex);

    ParallelFor(filmSize, [&](vec2 p) {
        RNG rng(Hash(p, sampleIndex));
        Ray ray = scene->camera.GenRay(
            (p - filmSize / 2 + rng.Uniform2f() - vec2(0.5f)) / filmSize.y, rng.Uniform2f());

        vec3 color = Li(ray, rng);
        film[p] = vec4(color, 1.0f) / (sampleIndex + 1) + film[p] * sampleIndex / (sampleIndex + 1);
    });

    if (++sampleIndex == samplePerPixel)
        pr.Report(sampleIndex);

    return sampleIndex < samplePerPixel;
}

}  // namespace pine