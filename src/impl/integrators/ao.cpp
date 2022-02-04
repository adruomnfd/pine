#include <impl/integrators/ao.h>
#include <core/scene.h>
#include <core/color.h>
#include <util/parallel.h>
#include <util/fileio.h>

namespace pine {

void AOIntegrator::Render() {
    Profiler _("Render");
    sampleIndex = 0;

    while (NextIteration()) {
    }

    ParallelFor(filmSize, [&](vec2i p) {
        film[p] = vec4(Pow(Uncharted2Flimic(film[p]), 1.0f / 2.2f), 1.0f);
    });
    SaveBMPImage(outputFileName, film.Size(), 4, (float*)film.Data());
}

bool AOIntegrator::NextIteration() {
    // std::vector<vec2> noise;
    // {
    //     SampledProfiler _(ProfilePhase::GenerateSamples);
    //     noise = GenerateBlueNoise(vec2i(128, 128), sampleIndex);
    // }

    pr.Report(sampleIndex);
    // int numSamplesSqrt = (int)sqrt(samplePerPixel);

    ParallelFor(filmSize, [&](vec2i p) {
        vec2i tileCo = {p.x % 120 + 2, p.y % 120 + 2};
        // vec2 s = vec2(sampleIndex % iS, sampleIndex / iS) / iS;
        // vec2 u2 = noise[tileCo.x + tileCo.y * 128] / iS + s;
        vec2 u2 = RNG(Hash(p, sampleIndex)).Uniform2f();
        Ray ray = scene->camera.GenRay(vec2(p - filmSize / 2) / filmSize.y, vec2(0.5f));

        vec3 L;
        Interaction it;
        if (Intersect(ray, it)) {
            ray.o = it.p;
            if (Dot(-ray.d, it.n) < 0.0f)
                it.n *= -1;
            ray.d = CoordinateSystem(it.n) * UniformHemisphereSampling(u2);
            ray.tmin = 1e-4f * ray.tmax;
            ray.tmax = FloatMax;
            if (!Intersect(ray, it))
                L = vec3(1.0f);
        }
        film[p] += vec4(L, 1.0f) / samplePerPixel;
    });

    if (++sampleIndex == samplePerPixel)
        pr.Report(sampleIndex);

    return sampleIndex < samplePerPixel;
}

}  // namespace pine