#include <impl/integrator/bdpt.h>
#include <core/scene.h>
#include <util/distribution.h>

#include <unordered_map>

namespace pine {

Spectrum Vertex::f(const Vertex &next) const {
    vec3 wo = Normalize(next.p() - p());
    switch (type) {
    case VertexType::Surface:
        return si.material->F(MaterialEvalCtx(p(), n(), si.uv, si.dpdu, si.dpdv, wi, wo));
    case VertexType::Medium: return si.phaseFunction->P(wi, wo);
    default: LOG_FATAL("[Vertex][f]Unimplemented"); return Spectrum(0.0f);
    }
}
Spectrum Vertex::Le(const Vertex &v) const {
    if (!IsLight())
        return Spectrum(0.0f);
    vec3 wo = Normalize(v.p() - p());
    return si.material->Le(MaterialEvalCtx(p(), n(), si.uv, si.dpdu, si.dpdv, wo));
}
float Vertex::ConvertDensity(float pdf, const Vertex &next) const {
    float dist;
    vec3 wo = Normalize(next.p() - p(), dist);
    if (next.IsOnSurface())
        pdf *= AbsDot(next.n(), wo);
    pdf /= pstd::sqr(dist);
    return pdf;
}
float Vertex::Pdf(const Vertex *prev, const Vertex &next) const {
    if (type == VertexType::Light)
        return PdfLight(next);
    vec3 wn = Normalize(next.p() - p());
    vec3 wp;
    if (prev)
        wp = Normalize(prev->p() - p());
    else
        CHECK(type == VertexType::Camera);

    float pdf = 0.0f;
    if (type == VertexType::Camera)
        pdf = ei.camera->PdfWe(ei.SpawnRay(wn)).dir;
    else if (type == VertexType::Surface)
        pdf = si.material->PDF(MaterialEvalCtx(p(), n(), si.uv, si.dpdu, si.dpdv, wp, wn));
    else if (type == VertexType::Medium)
        pdf = si.phaseFunction->P(wp, wn);
    else
        LOG_FATAL("[Vertex][Pdf]Unimplemented");

    return ConvertDensity(pdf, next);
}
float Vertex::PdfLight(const Vertex &v) const {
    CHECK(IsLight());
    float dist;
    vec3 w = Normalize(v.p() - p(), dist);
    float pdf = 0.0f;
    if (type == VertexType::Light)
        pdf = ei.light->PdfLe(Ray(p(), w)).dir;
    else
        pdf = 1.0f / Pi4;
    pdf /= pstd::sqr(dist);
    if (v.IsOnSurface())
        pdf *= AbsDot(v.n(), w);
    return pdf;
}
float Vertex::PdfLightOrigin(const Vertex &v) {
    vec3 w = Normalize(v.p() - p());
    float pdf;

    if (type == VertexType::Light)
        pdf = ei.light->PdfLe(Ray(p(), w)).pos;
    else
        pdf = 1.0f / si.shape->Area();
    return pdf;
}

template <typename T>
struct ScopedAssignment {
    ScopedAssignment(T *target = nullptr, T value = {}) : target(target) {
        if (target) {
            backup = *target;
            *target = value;
        }
    }
    ~ScopedAssignment() {
        if (target)
            *target = backup;
    }
    PINE_DELETE_COPY(ScopedAssignment)
    ScopedAssignment &operator=(ScopedAssignment &&other) {
        if (target)
            *target = backup;
        target = other.target;
        backup = other.backup;
        other.target = nullptr;
        return *this;
    }

    T *target, backup;
};

int RandomWalk(const Scene *, const RayIntegrator &integrator, Ray ray, Sampler &sampler,
               Spectrum beta, float pdf, int maxDepth, Vertex *path) {
    if (maxDepth == 0)
        return 0;
    int bounces = 0;
    float pdfFwd = pdf, pdfRev = 0.0f;

    while (true) {
        Interaction it;
        bool foundIntersection = integrator.Intersect(ray, it);
        if (ray.medium)
            beta *= ray.medium->Sample(ray, it, sampler);

        Vertex &vertex = path[bounces], &prev = path[bounces - 1];
        if (it.IsMediumInteraction()) {
            vertex = Vertex::CreateMedium(it, beta, pdfFwd, prev);
            if (++bounces >= maxDepth)
                break;

            vec3 wo;
            pdfFwd = pdfRev = it.phaseFunction->Sample(-ray.d, wo, sampler.Get2D());
            ray = it.SpawnRay(wo);
        } else {
            if (!foundIntersection) {
                break;
            }
            vertex = Vertex::CreateSurface(it, beta, pdfFwd, prev);
            if (++bounces >= maxDepth)
                break;

            if (!it.material) {
                ray = it.SpawnRay(ray.d);
                bounces--;
                continue;
            }

            it.n = it.material->BumpNormal(MaterialEvalCtx(it, -ray.d));
            auto mc = MaterialEvalCtx(it, -ray.d);
            mc.u2 = sampler.Get2D();
            mc.u1 = sampler.Get1D();

            auto bs = it.material->Sample(mc);
            if (!bs)
                break;
            pdfFwd = bs->pdf;
            beta *= bs->f * AbsDot(bs->wo, it.n) / pdfFwd;
            pdfRev = it.material->PDF(MaterialEvalCtx(it, bs->wo, -ray.d));
            ray = it.SpawnRay(bs->wo);
        }

        prev.pdfRev = vertex.ConvertDensity(pdfRev, prev);
    }

    return bounces;
}

int GenerateCameraSubpath(const Scene *scene, const RayIntegrator &integrator, Sampler &sampler,
                          int maxDepth, vec2 pFilm, Vertex *path) {
    if (maxDepth == 0)
        return 0;
    Ray ray = scene->camera.GenRay(pFilm, sampler.Get2D());
    Spectrum beta(1.0f);

    path[0] = Vertex::CreateCamera(&scene->camera, ray, beta);
    auto pdf = scene->camera.PdfWe(ray);
    return RandomWalk(scene, integrator, ray, sampler, beta, pdf.dir, maxDepth - 1, path + 1) + 1;
}

int GenerateLightSubpath(const Scene *scene, const RayIntegrator &integrator, Sampler &sampler,
                         int maxDepth, Vertex *path) {
    if (maxDepth == 0)
        return 0;
    auto [light, lightPdf] = integrator.lightSampler.SampleLight(sampler.Get1D());

    auto les = light->SampleLe(sampler.Get2D(), sampler.Get2D());
    path[0] = Vertex::CreateLight(light, les.ray, vec3(), les.Le, les.pdf.pos * lightPdf);
    Spectrum beta = les.Le / (les.pdf.pos * les.pdf.dir * lightPdf);

    int nVertices =
        RandomWalk(scene, integrator, les.ray, sampler, beta, les.pdf.dir, maxDepth - 1, path + 1);

    return nVertices + 1;
}

Spectrum G(const RayIntegrator &integrator, Sampler &sampler, const Vertex &v0, const Vertex &v1) {
    vec3 d = v0.p() - v1.p();
    float g = 1.0f / LengthSquared(d);
    d *= pstd::sqrt(g);
    if (v0.IsOnSurface())
        g *= AbsDot(v0.n(), d);
    if (v1.IsOnSurface())
        g *= AbsDot(v1.n(), d);
    return g * integrator.IntersectTr(RayBetween(v0.p(), v1.p()), sampler);
}

float MISWeight(Vertex *lightVertices, Vertex *cameraVertices, Vertex &sampled, int s, int t) {
    if (s + t == 2)
        return 1;
    float sumRi = 0.0f;
    auto remap0 = [](float f) { return f != 0.0f ? f : 1.0f; };

    Vertex *qs = s > 0 ? &lightVertices[s - 1] : nullptr,
           *pt = t > 0 ? &cameraVertices[t - 1] : nullptr,
           *qsMinus = s > 1 ? &lightVertices[s - 2] : nullptr,
           *ptMinus = t > 1 ? &cameraVertices[t - 2] : nullptr;

    ScopedAssignment<Vertex> a1;
    if (s == 1)
        a1 = {qs, sampled};
    else if (t == 1)
        a1 = {pt, sampled};

    ScopedAssignment<bool> a2, a3;
    if (pt)
        a2 = {&pt->delta, false};
    if (qs)
        a3 = {&qs->delta, false};

    ScopedAssignment<float> a4;
    if (pt)
        a4 = {&pt->pdfRev, s > 0 ? qs->Pdf(qsMinus, *pt) : pt->PdfLightOrigin(*ptMinus)};
    ScopedAssignment<float> a5;
    if (ptMinus)
        a5 = {&ptMinus->pdfRev, s > 0.0f ? pt->Pdf(qs, *ptMinus) : pt->PdfLight(*ptMinus)};

    ScopedAssignment<float> a6;
    if (qs)
        a6 = {&qs->pdfRev, pt->Pdf(ptMinus, *qs)};
    ScopedAssignment<float> a7;
    if (qsMinus)
        a7 = {&qsMinus->pdfRev, qs->Pdf(pt, *qsMinus)};

    float ri = 1.0f;
    for (int i = t - 1; i > 0; i--) {
        ri *= remap0(cameraVertices[i].pdfRev) / remap0(cameraVertices[i].pdfFwd);
        if (!cameraVertices[i].delta && !cameraVertices[i - 1].delta)
            sumRi += ri;
    }

    ri = 1.0f;
    for (int i = s - 1; i >= 0; i--) {
        ri *= remap0(lightVertices[i].pdfRev) / remap0(lightVertices[i].pdfFwd);
        bool deltaLightVertex =
            i > 0 ? lightVertices[i - 1].delta : lightVertices[0].IsDeltaLight();
        if (!lightVertices[i].delta && !deltaLightVertex)
            sumRi += ri;
    }
    return 1.0f / (1.0f + sumRi);
}

Spectrum ConnectBDPT(const RayIntegrator &integrator, Vertex *lightVertices, Vertex *cameraVertices,
                     int s, int t, const Camera &camera, Sampler &sampler, vec2 &pFilm) {
    Spectrum L(0.0f);
    if (t > 1 && s != 0 && cameraVertices[t - 1].type == VertexType::Light)
        return Spectrum(0.0f);

    Vertex sampled;
    if (s == 0) {
        const Vertex &pt = cameraVertices[t - 1];
        if (pt.IsLight())
            L = pt.Le(cameraVertices[t - 2]) * pt.beta;
    } else if (t == 1) {
        const Vertex &qs = lightVertices[s - 1];
        if (qs.IsConnectible()) {
            auto cs = camera.SampleWi(qs.p(), sampler.Get2D());
            pFilm = cs.pFilm;
            if (!cs.we.IsBlack()) {
                sampled = Vertex::CreateCamera(&camera, RayBetween(cs.p, qs.p()), cs.we / cs.pdf);
                L = qs.beta * qs.f(sampled) * sampled.beta;
                if (qs.IsOnSurface())
                    L *= AbsDot(cs.wo, qs.n());
                if (!L.IsBlack())
                    L *= integrator.IntersectTr(RayBetween(qs.p(), cs.p), sampler);
            }
        }
    } else if (s == 1) {
        const Vertex &pt = cameraVertices[t - 1];
        if (pt.IsConnectible()) {
            auto [light, lightPdf] = integrator.lightSampler.SampleLight(sampler.Get1D());
            auto ls = light->Sample(pt.p(), sampler.Get2D());
            Interaction it;
            it.p = pt.p() + ls.wo * ls.distance;
            it.wi = -ls.wo;
            EndpointInteraction ei(it, light);
            sampled = Vertex::CreateLight(ei, ls.Le / (lightPdf * ls.pdf), 0.0f);
            sampled.pdfFwd = sampled.PdfLightOrigin(pt);
            L = pt.beta * pt.f(sampled) * sampled.beta;
            if (pt.IsOnSurface())
                L *= AbsDot(ls.wo, pt.n());
            if (!L.IsBlack())
                L *= integrator.IntersectTr(RayBetween(pt.p(), it.p), sampler);
        }
    } else {
        const Vertex &qs = lightVertices[s - 1], &pt = cameraVertices[t - 1];
        if (qs.IsConnectible() && pt.IsConnectible()) {
            L = qs.beta * qs.f(pt) * pt.f(qs) * pt.beta;
            if (!L.IsBlack())
                L *= G(integrator, sampler, qs, pt);
        }
    }

    float misWeight = L.IsBlack() ? 0.0f : MISWeight(lightVertices, cameraVertices, sampled, s, t);
    L *= misWeight;

    return L;
}

BDPTIntegrator::BDPTIntegrator(const Parameters &params, Scene *scene)
    : PixelIntegrator(params, scene) {
    cameraVertices =
        pstd::vector<pstd::vector<Vertex>>(NumThreads(), pstd::vector<Vertex>(maxDepth + 2));
    lightVertices =
        pstd::vector<pstd::vector<Vertex>>(NumThreads(), pstd::vector<Vertex>(maxDepth + 1));
}

void BDPTIntegrator::Compute(vec2i p, Sampler &sampler) {
    vec2 pFilm = (p + sampler.Get2D()) / filmSize;
    auto cameraVertices = (Vertex *)&this->cameraVertices[threadIdx][0];
    auto lightVertices = (Vertex *)&this->lightVertices[threadIdx][0];
    int nCamera = GenerateCameraSubpath(scene, *this, sampler, maxDepth + 2, pFilm, cameraVertices);
    int nLight = GenerateLightSubpath(scene, *this, sampler, maxDepth + 1, lightVertices);
    Spectrum L(0.0f);

    for (int t = 1; t <= nCamera; t++) {
        for (int s = 0; s <= nLight; s++) {
            int depth = t + s - 2;
            if ((s == 1 && t == 1) || depth < 0 || depth > maxDepth)
                continue;

            vec2 pFilmNew = pFilm;
            Spectrum Lpath = ConnectBDPT(*this, lightVertices, cameraVertices, s, t, scene->camera,
                                         sampler, pFilmNew);
            if (Lpath.HasInfs() || Lpath.HasNaNs())
                continue;
            if (t != 1)
                L += Lpath;
            else
                film->AddSplat(pFilmNew, Lpath);
        }
    }
    film->AddSample(pFilm, L);
}

}  // namespace pine