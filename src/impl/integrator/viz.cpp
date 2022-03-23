#include <impl/integrator/viz.h>
#include <core/color.h>

namespace pine {

VizIntegrator::VizIntegrator(const Parameters& parameters, Scene* scene)
    : PixelSampleIntegrator(parameters, scene) {
    std::string viztype = parameters.GetString("viztype");
    SWITCH(viztype) {
        CASE("Bvh") type = Type::Bvh;
        CASE("Position") type = Type::Position;
        CASE("Normal") type = Type::Normal;
        CASE("Texcoord") type = Type::Texcoord;
        CASE("DpDu") type = Type::DpDu;
        CASE("DpDv") type = Type::DpDv;
        DEFAULT {
            LOG_WARNING("Unknown viztype &", viztype);
            type = Type::Normal;
        }
    }
};

Spectrum VizIntegrator::Li(Ray ray, Sampler&) {
    SampledProfiler _(ProfilePhase::EstimateLi);
    Interaction it;

    if (!Intersect(ray, it) && type != Type::Bvh)
        return Spectrum(0.0f);

    switch (type) {
    case Type::Bvh: return ColorMap(it.bvh / 200.0f);
    case Type::Position: return it.p;
    case Type::Normal: return it.n;
    case Type::Texcoord: return (vec3)it.uv;
    case Type::DpDu: return Abs(it.dpdu);
    case Type::DpDv: return Abs(it.dpdv);
    default: return vec3(1, 0, 1);
    }
}

}  // namespace pine