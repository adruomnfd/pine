#include <core/filter.h>
#include <util/parameters.h>

namespace pine {
Filter Filter::Create(const Parameters& params) {
    std::string type = params.GetString("type");
    vec2 radius = params.GetVec2("radius", vec2(1, 1));
    SWITCH(type) {
        CASE("Box") return new BoxFilter(radius);
        CASE("Triangle") return new TriangleFilter(radius);
        CASE("Gaussian") return new GaussianFilter(radius, params.GetFloat("alpha", 1.0f));
        CASE("Mitchell")
        return new MitchellFilter(radius, params.GetFloat("B", 1 / 3.0f),
                                  params.GetFloat("C", 1.0f / 3.0f));
        CASE("LanczosSinc") return new LanczosSincFilter(radius, params.GetFloat("tau", 3.0f));
        DEFAULT {
            LOG_WARNING("[Filter][Create]Unknown type \"&\"", type);
            return new BoxFilter(radius);
        }
    }
}
}  // namespace pine