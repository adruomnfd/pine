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

struct Vertex {
    Vertex() : ei(){};

    Vertex(VertexType type, const EndpointInteraction &ei, const Spectrum &beta)
        : type(type), beta(beta), ei(ei), wi(ei.wi) {
    }

    Vertex(VertexType type, const Interaction &si, const Spectrum &beta)
        : type(type), beta(beta), si(si), wi(si.wi) {
    }

    const Interaction &GetInteraction() const {
        switch (type) {
        case VertexType::Surface:
        case VertexType::Medium: return si;
        default: return ei;
        }
    }
    vec3 p() const {
        return GetInteraction().p;
    }
    vec3 n() const {
        return GetInteraction().n;
    }
    bool IsOnSurface() const {
        return n() != vec3(0.0f);
    }
    Spectrum f(const Vertex &next) const;

    bool IsConnectible() const {
        switch (type) {
        case VertexType::Medium: return true;
        case VertexType::Light: return !ei.light->IsDelta();
        case VertexType::Camera: return true;
        case VertexType::Surface: return true;
        default: break;
        }
        LOG_FATAL("[Vertex][IsConnectible]Unimplemented");
        return false;
    }

    bool IsLight() const {
        return type == VertexType::Light ||
               (type == VertexType::Surface && si.material->Is<EmissiveMaterial>());
    }
    bool IsDeltaLight() const {
        return type == VertexType::Light && ei.light && ei.light->IsDelta();
    }
    Spectrum Le(const Vertex &v) const;
    float ConvertDensity(float pdf, const Vertex &next) const;
    float Pdf(const Vertex *prev, const Vertex &next) const;
    float PdfLight(const Vertex &v) const;
    float PdfLightOrigin(const Vertex &v);

    static inline Vertex CreateCamera(const Camera *camera, const Ray &ray, const Spectrum &beta) {
        return Vertex(VertexType::Camera, EndpointInteraction(camera, ray), beta);
    }
    static inline Vertex CreateCamera(const Camera *camera, const Interaction &it,
                                      const Spectrum &beta) {
        return Vertex(VertexType::Camera, EndpointInteraction(it, camera), beta);
    }
    static inline Vertex CreateLight(const Light *light, const Ray &ray, const vec3 &nLight,
                                     const Spectrum &Le, float pdf) {
        Vertex v(VertexType::Light, EndpointInteraction(light, ray, nLight), Le);
        v.pdfFwd = pdf;
        return v;
    }
    static inline Vertex CreateLight(const EndpointInteraction &ei, const Spectrum &beta,
                                     float pdf) {
        Vertex v(VertexType::Light, ei, beta);
        v.pdfFwd = pdf;
        return v;
    }
    static inline Vertex CreateMedium(const Interaction &mi, const Spectrum &beta, float pdf,
                                      const Vertex &prev) {
        Vertex v(VertexType::Medium, mi, beta);
        v.pdfFwd = prev.ConvertDensity(pdf, v);
        return v;
    }
    static inline Vertex CreateSurface(const Interaction &si, const Spectrum &beta, float pdf,
                                       const Vertex &prev) {
        Vertex v(VertexType::Surface, si, beta);
        v.pdfFwd = prev.ConvertDensity(pdf, v);
        return v;
    }

    VertexType type = VertexType::Invalid;
    Spectrum beta;
    EndpointInteraction ei;
    Interaction si;
    vec3 wi;
    bool delta = false;
    float pdfFwd = 0.0f, pdfRev = 0.0f;
};

struct Vertex;

int RandomWalk(const Scene *, const RayIntegrator &integrator, Ray ray, Sampler &sampler,
               Spectrum beta, float pdf, int maxDepth, Vertex *path);

int GenerateCameraSubpath(const Scene *scene, const RayIntegrator &integrator, Sampler &sampler,
                          int maxDepth, vec2 pFilm, Vertex *path);

int GenerateLightSubpath(const Scene *scene, const RayIntegrator &integrator, Sampler &sampler,
                         int maxDepth, Vertex *path);

Spectrum G(const RayIntegrator &integrator, Sampler &sampler, const Vertex &v0, const Vertex &v1);

float MISWeight(Vertex *lightVertices, Vertex *cameraVertices, Vertex &sampled, int s, int t);

Spectrum ConnectBDPT(const RayIntegrator &integrator, Vertex *lightVertices, Vertex *cameraVertices,
                     int s, int t, const Camera &camera, Sampler &sampler, vec2 &pFilm);

class BDPTIntegrator : public PixelIntegrator {
  public:
    BDPTIntegrator(const Parameters &params, Scene *scene);
    void Compute(vec2i p, Sampler &sampler);

    pstd::vector<pstd::vector<Vertex>> cameraVertices;
    pstd::vector<pstd::vector<Vertex>> lightVertices;
};

}  // namespace pine

#endif  // PINE_IMPL_INTEGRATOR_BDPT_H