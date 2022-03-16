#include <core/integrator.h>
#include <core/sampling.h>
#include <core/scene.h>
#include <core/color.h>
#include <util/parallel.h>
#include <util/profiler.h>
#include <util/fileio.h>
#include <impl/integrators/ao.h>
#include <impl/integrators/viz.h>
#include <impl/integrators/path.h>

namespace pine {

std::shared_ptr<Integrator> Integrator::Create(const Parameters& parameters) {
    std::string type = parameters.GetString("type");
    SWITCH(type) {
        CASE("AO") return std::make_shared<AOIntegrator>(parameters);
        CASE("Path") return std::make_shared<PathIntegrator>(parameters);
        CASE("Viz") return std::make_shared<VizIntegrator>(parameters);
        DEFAULT {
            LOG_WARNING("[Integrator][Create]Unknown type \"&\"", type);
            return std::make_shared<AOIntegrator>(parameters);
        }
    }
}
Integrator::Integrator(const Parameters& parameters) {
    filmSize = parameters.GetVec2i("filmSize", vec2i(1280, 720));

    film = Image(filmSize);

    Parameters samplerParams = parameters["sampler"];
    samplerParams.Set("filmSize", filmSize);
    samplers = {Sampler::Create(samplerParams)};
    for (int i = 0; i < NumThreads() - 1; i++)
        samplers.push_back(samplers[0].Clone());
    samplesPerPixel = samplers[0].SamplesPerPixel();
}
void Integrator::Initialize(const Scene* scene) {
    Profiler _("IntegratorInit");
    this->scene = scene;
    outputFileName = scene->parameters.GetString("outputFileName", "result.bmp");
}

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
bool RayIntegrator::IntersectTr(Ray ray, vec3& tr, Sampler& sampler) {
    SampledProfiler _(ProfilePhase::IntersectTr);

    vec3 p2 = ray(ray.tmax);
    Interaction it;
    tr = vec3(1.0f);

    while (true) {
        bool hitSurface = Intersect(ray, it);
        if (ray.medium)
            tr *= ray.medium.Tr(ray, sampler);
        if (!hitSurface)
            return false;
        if (it.material)
            return true;
        ray = it.SpawnRayTo(p2);
    }
}
vec3 RayIntegrator::EstimateDirect(Ray ray, Interaction it, Sampler& sampler) {
    SampledProfiler _(ProfilePhase::EstimateDirect);

    LightSample ls;

    // float lightChoiceProb = scene->lights.size() ? (sunIntensity == 0.0f ? 1.0f : 0.5f) : 0.0f;

    // if (sampler.Get1D() < lightChoiceProb) {
    uint64_t lightIndex = sampler.Get1D() * scene->lights.size();
    ls = scene->lights[lightIndex].Sample(it.p, sampler.Get1D(), sampler.Get2D());
    ls.pdf = ls.pdf;  // / lightChoiceProb;
    // } else {
    //     ls.Le = AtmosphereColor(sunDirection, sunDirection, sunIntensity);
    //     ls.wo = sunDirection;
    //     ls.distance = 1e+10f;
    //     ls.p = ls.wo * ls.distance;
    //     ls.isDelta = true;
    //     ls.pdf = 1.0f / (1.0f - lightChoiceProb);
    // }

    vec3 tr = vec3(1.0f);
    if (IntersectTr(it.SpawnRayTo(ls.p), tr, sampler))
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

void PixelSampleIntegrator::Render() {
    Profiler _("Render");

    ProgressReporter pr("Rendering", "Samples", "Samples", samplesPerPixel,
                        filmSize.x * filmSize.y);

    for (int sampleIndex = 0; sampleIndex < samplesPerPixel; sampleIndex++) {
        ScopedPR(pr, sampleIndex);

        ThreadIdParallelFor(filmSize, [&](int id, vec2i p) {
            Sampler& sampler = samplers[id];
            sampler.StartPixel(p, sampleIndex);
            Ray ray = scene->camera.GenRay(
                (p - filmSize / 2 + sampler.Get2D() - vec2(0.5f)) / filmSize.y, sampler.Get2D());

            vec3 color = Li(ray, sampler);
            film[p] =
                vec4(color, 1.0f) / (sampleIndex + 1) + film[p] * sampleIndex / (sampleIndex + 1);
        });
    }

    ParallelFor(filmSize, [&](vec2i p) {
        film[p] = vec4(Pow(Uncharted2Flimic(film[p]), 1.0f / 2.2f), 1.0f);
    });
    SaveBMPImage(outputFileName, film.Size(), 4, (float*)film.Data());
}

void SinglePassIntegrator::Render() {
    Profiler _("Render");

    int total = filmSize.x * filmSize.y;
    int groupSize = max(total / 100, 1);
    int nGroups = (total + groupSize - 1) / groupSize;

    ProgressReporter pr("Rendering", "Pixels", "Pixels", total, total);

    for (int i = 0; i < nGroups; i++) {
        ScopedPR(pr, i * groupSize);

        ThreadIdParallelFor(groupSize, [&](int id, int index) {
            index += i * groupSize;
            if (index >= total)
                return;
            vec2i p = {index % filmSize.x, index / filmSize.x};

            Sampler& sampler = samplers[id];
            sampler.StartPixel(p, 0);

            vec3 color;
            for (int sampleIndex = 0; sampleIndex < samplesPerPixel; sampleIndex++) {
                Ray ray = scene->camera.GenRay(
                    (p - filmSize / 2 + sampler.Get2D() - vec2(0.5f)) / filmSize.y,
                    sampler.Get2D());
                color += Li(ray, sampler);
                sampler.StartNextSample();
            }

            color /= samplesPerPixel;
            color = Pow(Uncharted2Flimic(color), 1.0f / 2.2f);
            film[p] = vec4(color, 1.0f);
        });
    }

    SaveBMPImage(outputFileName, film.Size(), 4, (float*)film.Data());
}

}  // namespace pine