#ifndef PINE_CORE_SCENE_H
#define PINE_CORE_SCENE_H

#include <core/integrator.h>
#include <core/geometry.h>
#include <core/light.h>
#include <core/medium.h>
#include <core/camera.h>
#include <util/parameters.h>

#include <pstd/optional.h>
#include <pstd/memory.h>
#include <pstd/vector.h>
#include <pstd/map.h>

namespace pine {

struct Scene {
    pstd::shared_ptr<Integrator> integrator;

    pstd::map<std::string, std::shared_ptr<Material>> materials;
    pstd::map<std::string, std::shared_ptr<Medium>> mediums;
    pstd::vector<Shape> shapes;
    pstd::vector<Light> lights;
    pstd::optional<EnvironmentLight> envLight;
    Camera camera;
};

}  // namespace pine

#endif  // PINE_CORE_SCENE_H