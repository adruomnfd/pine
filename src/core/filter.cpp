#include <core/filter.h>
#include <util/parameters.h>

namespace pine {
Filter CreateFilter(const Parameters& params) {
    std::string type = params.GetString("type", "LanczosSinc");
    vec2 radius = params.GetVec2("radius", vec2(0.5f));
    SWITCH(type) {
        CASE("Box") return BoxFilter(radius);
        CASE("Triangle") return TriangleFilter(radius);
        CASE("Gaussian") return GaussianFilter(radius, params.GetFloat("alpha", 1.0f));
        CASE("Mitchell")
        return MitchellFilter(radius, params.GetFloat("B", 1 / 3.0f),
                              params.GetFloat("C", 1.0f / 3.0f));
        CASE("LanczosSinc") return LanczosSincFilter(radius, params.GetFloat("tau", 3.0f));
        DEFAULT {
            LOG_WARNING("[Filter][Create]Unknown type \"&\"", type);
            return BoxFilter(radius);
        }
    }
}
}  // namespace pine