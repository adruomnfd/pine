#include <core/accel.h>
#include <util/parameters.h>

#include <impl/accel/bvh.h>
#include <impl/accel/cwbvh.h>

namespace pine {

Accel* CreateAccel(const Parameters& params) {
    pstd::string type = params.GetString("type", "BVH");
    SWITCH(type) {
        CASE("BVH") return new BVH(params);
        // CASE("CWBVH") return new CWBVH(params);
        DEFAULT {
            LOG_WARNING("[Accel][Create]Unknown type \"&\"", type);
            return new BVH(params);
        }
    }
}

}  // namespace pine