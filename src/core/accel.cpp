#include <core/accel.h>
#include <util/parameters.h>

#include <impl/accel/bvh.h>
#include <impl/accel/cwbvh.h>

namespace pine {

std::shared_ptr<Accel> Accel::Create(const Parameters& params) {
    std::string type = params.GetString("type", "BVH");
    SWITCH(type) {
        CASE("BVH") return std::make_shared<BVH>(params);
        // CASE("CWBVH") return std::make_shared<CWBVH>(params);
        DEFAULT {
            LOG_WARNING("[Accel][Create]Unknown type \"&\"", type);
            return std::make_shared<BVH>(params);
        }
    }
}

}  // namespace pine