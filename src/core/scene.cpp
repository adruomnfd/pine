#include <core/scene.h>
#include <core/integrator.h>

namespace pine {

void Scene::Initialize() {
    CHECK(integrator != nullptr);

    int lightId = 0;
    for (Light light : lights) {
        if (light.Tag() == Light::type::Index<AreaLight>()) {
            AreaLight* areaLight = (AreaLight*)light.Ptr();
            Parameters materialParams;
            std::string materialName = "areaLightMaterial" + ToString(lightId++);
            materialParams.Set("type", "EmissiveMaterial");
            materialParams["color"].Set("type", "Constant");
            materialParams["color"].Set("vec3", areaLight->color);
            materials[materialName] = Material::Create(materialParams);

            Parameters shapeParams;
            shapeParams.Set("type", "Rect");
            shapeParams.Set("position", areaLight->position);
            shapeParams.Set("ex", areaLight->ex);
            shapeParams.Set("ey", areaLight->ey);
            shapeParams.Set("material", materialName);
            shapes.push_back(Shape::Create(shapeParams, this));
        }
    }

    integrator->Initialize(this);
}
void Scene::Cleanup() {
    Camera::Destory(camera);
    for (auto& shape : shapes)
        Shape::Destory(shape);
    for (auto& light : lights)
        Light::Destory(light);
    for (auto& medium : mediums)
        Medium::Destory(medium.second);
    for (auto& material : materials)
        Material::Destory(material.second);
}

}  // namespace pine