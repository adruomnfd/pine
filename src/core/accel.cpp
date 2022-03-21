#include <core/accel.h>
#include <util/parameters.h>

#include <impl/accel/bvh.h>
#include <impl/accel/cwbvh.h>

namespace pine {

std::shared_ptr<Accel> Accel::Create(const Parameters& parameters) {
    std::string type = parameters.GetString("type");
    SWITCH(type) {
        CASE("BVH") return std::make_shared<BVH>(parameters);
        // CASE("CWBVH") return std::make_shared<CWBVH>(parameters);
        DEFAULT {
            LOG_WARNING("[Accel][Create]Unknown type \"&\"", type);
            return std::make_shared<BVH>(parameters);
        }
    }
}

}  // namespace pine