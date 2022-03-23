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
#include <impl/integrator/lightpath.h>

namespace pine {

std::shared_ptr<Integrator> Integrator::Create(const Parameters& parameters, Scene* scene) {
    std::string type = parameters.GetString("type");
    SWITCH(type) {
        CASE("AO") return std::make_shared<AOIntegrator>(parameters, scene);
        CASE("Viz") return std::make_shared<VizIntegrator>(parameters, scene);
        CASE("Mlt") return std::make_shared<MltIntegrator>(parameters, scene);
        CASE("Path") return std::make_shared<PathIntegrator>(parameters, scene);
        CASE("LightPath") return std::make_shared<LightPathIntegrator>(parameters, scene);
        DEFAULT {
            LOG_WARNING("[Integrator][Create]Unknown type \"&\"", type);
            return std::make_shared<PathIntegrator>(parameters, scene);
        }
    }
}
Integrator::Integrator(const Parameters& parameters, Scene* scene) : scene(scene) {
    film = &scene->camera.GetFilm();
    filmSize = scene->camera.GetFilm().Size();

    lightSampler = LightSampler::Create(parameters["lightSampler"], scene->lights);

    Parameters samplerParams = parameters["sampler"];
    samplerParams.Set("filmSize", filmSize);
    samplers = {Sampler::Create(samplerParams)};
    for (int i = 0; i < NumThreads() - 1; i++)
        samplers.push_back(samplers[0].Clone());
    samplesPerPixel = samplers[0].SamplesPerPixel();
}

RayIntegrator::RayIntegrator(const Parameters& parameters, Scene* scene)
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
Spectrum RayIntegrator::IntersectTr(Ray ray, Sampler& sampler) {
    SampledProfiler _(ProfilePhase::IntersectTr);

    vec3 p2 = ray(ray.tmax);
    Interaction it;
    Spectrum tr = Spectrum(1.0f);

    while (true) {
        bool hitSurface = Intersect(ray, it);
        if (ray.medium)
            tr *= ray.medium->Tr(ray, sampler);
        if (hitSurface && it.material)
            return Spectrum(0.0f);
        if (!hitSurface)
            break;
        ray = it.SpawnRayTo(p2);
    }

    return tr;
}
Spectrum RayIntegrator::EstimateDirect(Ray ray, Interaction it, Sampler& sampler) {
    SampledProfiler _(ProfilePhase::EstimateDirect);

    auto [light, slPdf] = lightSampler.SampleLight(it.p, it.n, sampler.Get1D());
    if (!light)
        return Spectrum(0.0f);
    LightSample ls = light->Sample(it.p, sampler.Get2D());
    ls.pdf *= slPdf;

    Spectrum tr = IntersectTr(it.SpawnRay(ls.wo, ls.distance), sampler);
    if (tr.IsBlack())
        return Spectrum(0.0f);

    Spectrum f;
    float scatteringPdf;
    if (it.IsSurfaceInteraction()) {
        MaterialEvalContext mc(it.p, it.n, it.uv, it.dpdu, it.dpdv, -ray.d, ls.wo);
        f = it.material->F(mc) * AbsDot(ls.wo, it.n);
        scatteringPdf = it.material->PDF(mc);
    } else {
        float p = it.phaseFunction->P(-ray.d, ls.wo);
        f = Spectrum(p);
        scatteringPdf = p;
    }
    float w = PowerHeuristic(1, ls.pdf, 1, scatteringPdf);

    if (ls.isDelta)
        return tr * f * ls.Le / ls.pdf;
    else
        return tr * f * w * ls.Le / ls.pdf;
}

void PixelSampleIntegrator::Render() {
    Profiler _("Render");
    film->Clear();

    ProgressReporter pr("Rendering", "Samples", "Samples", samplesPerPixel, Area(filmSize));

    for (int sampleIndex = 0; sampleIndex < samplesPerPixel; sampleIndex++) {
        ScopedPR(pr, sampleIndex, sampleIndex + 1 == samplesPerPixel);

        ThreadIdParallelFor(filmSize, [&](int id, vec2i p) {
            Sampler& sampler = samplers[id];
            sampler.StartPixel(p, sampleIndex);

            vec2 pFilm = (p + sampler.Get2D()) / film->Size();
            Ray ray = scene->camera.GenRay(pFilm, sampler.Get2D());

            film->AddSample(pFilm, Li(ray, sampler));
        });
    }

    film->Finalize();
}

void SinglePassIntegrator::Render() {
    Profiler _("Render");
    film->Clear();

    int total = Area(filmSize);
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
                vec2 pFilm = (p + sampler.Get2D()) / film->Size();
                Ray ray = scene->camera.GenRay(pFilm, sampler.Get2D());
                film->AddSample(pFilm, Li(ray, sampler));
                sampler.StartNextSample();
            }
        });
    }

    film->Finalize();
}

}  // namespace pine