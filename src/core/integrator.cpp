#include <core/integrator.h>
#include <core/sampling.h>
#include <core/scene.h>
#include <core/color.h>
#include <util/parallel.h>
#include <util/profiler.h>
#include <util/fileio.h>
#include <impl/integrator/ao.h>
#include <impl/integrator/viz.h>
#include <impl/integrator/mlt.h>
#include <impl/integrator/path.h>

namespace pine {

std::shared_ptr<Integrator> Integrator::Create(const Parameters& parameters, const Scene* scene) {
    std::string type = parameters.GetString("type");
    SWITCH(type) {
        CASE("AO") return std::make_shared<AOIntegrator>(parameters, scene);
        CASE("Viz") return std::make_shared<VizIntegrator>(parameters, scene);
        CASE("Mlt") return std::make_shared<MltIntegrator>(parameters, scene);
        CASE("Path") return std::make_shared<PathIntegrator>(parameters, scene);
        DEFAULT {
            LOG_WARNING("[Integrator][Create]Unknown type \"&\"", type);
            return std::make_shared<AOIntegrator>(parameters, scene);
        }
    }
}
Integrator::Integrator(const Parameters& parameters, const Scene* scene) : scene(scene) {
    film = Film::Create(parameters["film"]);
    filmSize = film.Size();

    lightSampler = LightSampler::Create(parameters["lightSampler"], scene->lights);

    Parameters samplerParams = parameters["sampler"];
    samplerParams.Set("filmSize", filmSize);
    samplers = {Sampler::Create(samplerParams)};
    for (int i = 0; i < NumThreads() - 1; i++)
        samplers.push_back(samplers[0].Clone());
    samplesPerPixel = samplers[0].SamplesPerPixel();
}

RayIntegrator::RayIntegrator(const Parameters& parameters, const Scene* scene)
    : Integrator(parameters, scene), accel(Accel::Create(parameters["accel"])) {
    accel->Initialize(scene);
}
bool RayIntegrator::Hit(Ray ray) {
    SampledProfiler _(ProfilePhase::IntersectShadow);

    Interaction it;
    return accel->Intersect(ray, it);
}
bool RayIntegrator::Intersect(Ray& ray, Interaction& it) {
    SampledProfiler _(ProfilePhase::IntersectClosest);

    return accel->Intersect(ray, it);
}
bool RayIntegrator::IntersectTr(Ray ray, Spectrum& tr, Sampler& sampler) {
    SampledProfiler _(ProfilePhase::IntersectTr);

    vec3 p2 = ray(ray.tmax);
    Interaction it;
    tr = Spectrum(1.0f);

    while (true) {
        bool hitSurface = Intersect(ray, it);
        if (ray.medium)
            tr *= ray.medium->Tr(ray, sampler);
        if (!hitSurface)
            return false;
        if (it.material)
            return true;
        ray = it.SpawnRayTo(p2);
    }
}
Spectrum RayIntegrator::EstimateDirect(Ray ray, Interaction it, Sampler& sampler) {
    SampledProfiler _(ProfilePhase::EstimateDirect);

    LightSample ls = lightSampler.Sample(it.p, sampler.Get1D(), sampler.Get2D());

    Spectrum tr = Spectrum(1.0f);
    if (IntersectTr(it.SpawnRay(ls.wo, ls.distance), tr, sampler))
        return Spectrum(0.0f);

    if (it.IsSurfaceInteraction()) {
        MaterialEvalContext mc(it.p, it.n, it.uv, it.dpdu, it.dpdv, -ray.d, ls.wo);
        Spectrum f = it.material->F(mc);
        float pdf = it.material->PDF(mc);
        float w = PowerHeuristic(1, ls.pdf, 1, pdf);
        return tr * f * AbsDot(ls.wo, it.n) * w * ls.Le / ls.pdf;
    } else {
        return tr * it.phaseFunction.F(-ray.d, ls.wo) * ls.Le / ls.pdf;
    }
}

void PixelSampleIntegrator::Render() {
    Profiler _("Render");
    film.Clear();

    ProgressReporter pr("Rendering", "Samples", "Samples", samplesPerPixel,
                        filmSize.x * filmSize.y);

    for (int sampleIndex = 0; sampleIndex < samplesPerPixel; sampleIndex++) {
        ScopedPR(pr, sampleIndex, sampleIndex + 1 == samplesPerPixel);

        ThreadIdParallelFor(filmSize, [&](int id, vec2i p) {
            Sampler& sampler = samplers[id];
            sampler.StartPixel(p, sampleIndex);

            vec2 pFilm = p + sampler.Get2D();
            Ray ray = scene->camera.GenRay(filmSize, pFilm, sampler.Get2D());

            film.AddSample(pFilm, Li(ray, sampler));
        });
    }

    film.Finalize();
}

void SinglePassIntegrator::Render() {
    Profiler _("Render");
    film.Clear();

    int total = filmSize.x * filmSize.y;
    int groupSize = max(total / 100, 1);
    int nGroups = (total + groupSize - 1) / groupSize;

    ProgressReporter pr("Rendering", "Pixels", "Samples", total, samplesPerPixel);

    for (int i = 0; i < nGroups; i++) {
        ScopedPR(pr, i * groupSize, i + 1 == nGroups);

        ThreadIdParallelFor(groupSize, [&](int id, int index) {
            index += i * groupSize;
            if (index >= total)
                return;
            vec2i p = {index % filmSize.x, index / filmSize.x};

            Sampler& sampler = samplers[id];
            sampler.StartPixel(p, 0);

            for (int sampleIndex = 0; sampleIndex < samplesPerPixel; sampleIndex++) {
                vec2 pFilm = p + sampler.Get2D();
                Ray ray = scene->camera.GenRay(filmSize, pFilm, sampler.Get2D());
                film.AddSample(pFilm, Li(ray, sampler));
                sampler.StartNextSample();
            }
        });
    }

    film.Finalize();
}

}  // namespace pine