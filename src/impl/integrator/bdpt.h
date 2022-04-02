#ifndef PINE_IMPL_INTEGRATOR_BDPT_H
#define PINE_IMPL_INTEGRATOR_BDPT_H

#include <core/integrator.h>

namespace pine {

struct EndpointInteraction : Interaction {
    const Camera *camera = nullptr;
    const Light *light = nullptr;

    EndpointInteraction() : Interaction(), light(nullptr){};

    EndpointInteraction(const Interaction &it, const Camera *camera)
        : Interaction(it), camera(camera){};

    EndpointInteraction(const Interaction &it, const Light *light)
        : Interaction(it), light(light){};

    EndpointInteraction(const Camera *camera, const Ray &ray) : camera(camera) {
        p = ray.o;
        mediumInterface = ray.medium;
    };

    EndpointInteraction(const Light *light, const Ray &ray, vec3 nl) : light(light) {
        p = ray.o;
        n = nl;
        mediumInterface = ray.medium;
    };

    EndpointInteraction(const Ray &ray) {
        p = ray(1.0f);
        mediumInterface = ray.medium;
        n = -ray.d;
    };
};

enum class VertexType { Camera, Light, Surface, Medium, Invalid };

struct VertexBase {
    VertexBase() : ei(){};

    VertexBase(VertexType type, const EndpointInteraction &ei, const Spectrum &beta)
        : type(type), beta(beta), ei(ei), wi(ei.wi) {
    }

    VertexBase(VertexType type, const Interaction &si, const Spectrum &beta)
        : type(type), beta(beta), si(si), wi(si.wi) {
    }

    VertexType type = VertexType::Invalid;
    Spectrum beta;
    EndpointInteraction ei;
    Interaction si;
    vec3 wi;
    bool delta = false;
    float pdfFwd = 0.0f, pdfRev = 0.0f;
};

class BDPTIntegrator : public PixelIntegrator {
  public:
    BDPTIntegrator(const Parameters &params, Scene *scene);
    void Compute(vec2i p, Sampler &sampler);

  private:
    std::vector<std::vector<VertexBase>> cameraVertices;
    std::vector<std::vector<VertexBase>> lightVertices;
};

}  // namespace pine

#endif  // PINE_IMPL_INTEGRATOR_BDPT_H