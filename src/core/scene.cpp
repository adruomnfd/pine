#include <core/scene.h>
#include <core/integrator.h>

namespace pine {

void Scene::Initialize() {
    CHECK(integrator != nullptr);

    int lightId = 0;
    for (const Light& light : lights) {
        if (light.Tag() == Light::Index<AreaLight>()) {
            const AreaLight& areaLight = light.Be<AreaLight>();
            Parameters materialParams;
            std::string materialName = "areaLightMaterial" + ToString(lightId++);
            materialParams.Set("type", "Emissive");
            materialParams["color"].Set("type", "Constant");
            materialParams["color"].Set("vec3", areaLight.color.ToRGB());
            materials[materialName] = Material::Create(materialParams);

            Parameters shapeParams;
            shapeParams.Set("type", "Rect");
            shapeParams.Set("position", areaLight.position);
            shapeParams.Set("ex", areaLight.ex);
            shapeParams.Set("ey", areaLight.ey);
            shapeParams.Set("material", materialName);
            shapes.push_back(Shape::Create(shapeParams, this));
        }
        if (light.Tag() == Light::Index<EnvironmentLight>()) {
            envLight = light.Be<EnvironmentLight>();
        }
    }
}
void Scene::Cleanup() {
    for (auto& medium : mediums)
        Medium::Destory(medium.second);
}

}  // namespace pine