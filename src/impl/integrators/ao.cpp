#include <impl/integrators/ao.h>
#include <core/scene.h>
#include <core/color.h>
#include <util/parallel.h>
#include <util/fileio.h>

namespace pine {

vec3 AOIntegrator::Li(Ray ray, Sampler& sampler) {
    vec3 color;
    Interaction it;
    if (Intersect(ray, it)) {
        ray.o = it.p;
        if (Dot(-ray.d, it.n) < 0.0f)
            it.n *= -1;
        ray.d = CoordinateSystem(it.n) * UniformHemisphereSampling(sampler.Get2D());
        ray.tmin = 1e-4f * ray.tmax;
        ray.tmax = FloatMax;
        if (!Intersect(ray, it))
            color = vec3(1.0f);
    }
    return color;
}

}  // namespace pine