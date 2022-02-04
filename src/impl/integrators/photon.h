#ifndef PINE_IMPL_PHOTON_INTEGRATOR_H
#define PINE_IMPL_PHOTON_INTEGRATOR_H

#include <core/integrator.h>

#include <queue>

namespace pine {

// Impl reference: https://github.com/yumcyaWiz/photon_mapping
struct Photon {
    Photon() = default;
    Photon(vec3 flux, vec3 position, vec3 wi) : flux(flux), position(position), wi(wi){};
    float operator[](int i) const {
        return position[i];
    }
    vec3 flux;
    vec3 position;
    vec3 wi;
};
struct PhotonMap {
    struct Node {
        uint32_t axis = -1;
        uint32_t index = -1;
        uint32_t leftChildIndex = -1, rightChildIndex = -1;
    };
    struct Neighbor {
        Neighbor() = default;
        Neighbor(uint32_t photonIndex, float distance)
            : photonIndex(photonIndex), distance(distance) {
        }
        friend bool operator<(Neighbor lhs, Neighbor rhs) {
            return lhs.distance < rhs.distance;
        }
        uint32_t photonIndex = -1;
        float distance = 0.0f;
    };
    struct Neighbors {
        std::vector<Neighbor> neighbors;
        float maxDistance = 0.0f;
    };
    using Queue = std::priority_queue<Neighbor>;

    void Build();
    Neighbors SearchNeighbors(vec3 position, uint32_t numNeighbors) const;

    uint32_t NumPhotons() const {
        return photons.size();
    }
    void AddPhoton(const Photon& photon) {
        photons.push_back(photon);
    }
    const Photon& GetPhoton(int i) const {
        return photons[i];
    }

  private:
    uint32_t BuildNode(uint32_t* indices, uint32_t size, uint32_t depth);
    void SearchNode(uint32_t nodeIndex, vec3 point, uint32_t k, Queue& queue) const;

    std::vector<Node> nodes;
    std::vector<Photon> photons;
};

class PhotonIntegrator : public RayIntegrator {
  public:
    PhotonIntegrator(const Parameters& parameters);

    void Render() override;
    bool NextIteration() override;

    vec3 Radiance(vec3 wo, Interaction it) const;

    int maxDepth;
    bool visualizePhoton;

    int numPhotons;
    int numPhotonsEstimation;
    int finalGatheringDepth;
    PhotonMap photonMap;
};

}  // namespace pine

#endif  // PINE_IMPL_PHOTON_INTEGRATOR_H