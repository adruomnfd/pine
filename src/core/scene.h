#ifndef PINE_CORE_SCENE_H
#define PINE_CORE_SCENE_H

#include <core/geometry.h>
#include <core/light.h>
#include <core/medium.h>
#include <core/camera.h>
#include <core/integrator.h>
#include <util/parameters.h>

#include <map>
#include <vector>
#include <memory>

namespace pine {

struct Scene {
    void Initialize();
    void Cleanup();

    std::shared_ptr<Integrator> integrator;

    std::map<std::string, Material> materials;
    std::map<std::string, Medium> mediums;
    std::map<std::string, uint32_t> nameToMeshIndex;
    std::vector<Shape> shapes;
    std::vector<TriangleMesh> meshes;
    std::vector<Light> lights;

    vec3 sunDirection;
    float sunIntensity;

    Camera camera;
    Parameters parameters;
};

}  // namespace pine

#endif  // PINE_CORE_SCENE_H