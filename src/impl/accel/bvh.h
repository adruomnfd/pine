#ifndef PINE_IMPL_ACCEL_BVH_H
#define PINE_IMPL_ACCEL_BVH_H

#include <core/accel.h>

#include <memory>
#include <vector>

namespace pine {

class BVHImpl {
  public:
    struct alignas(16) Node {
        float SurfaceArea() const {
            return Union(aabbs[0], aabbs[1]).SurfaceArea();
        }
        AABB GetAABB() const {
            return Union(aabbs[0], aabbs[1]);
        }
        void UpdateAABB(Node* nodes) {
            aabbs[0] = nodes[children[0]].GetAABB();
            aabbs[1] = nodes[children[1]].GetAABB();
            if (parent != -1)
                nodes[parent].UpdateAABB(nodes);
        }
        float ComputeCost(Node* nodes) {
            if (primitiveIndices.size())
                return SurfaceArea();
            return SurfaceArea() + nodes[children[0]].ComputeCost(nodes) +
                   nodes[children[1]].ComputeCost(nodes);
        }
        float Inefficiency() const {
            float mSum = SurfaceArea() / (2 * (aabbs[0].SurfaceArea() + aabbs[1].SurfaceArea()));
            float mMin = SurfaceArea() / std::min(aabbs[0].SurfaceArea(), aabbs[1].SurfaceArea());
            float mArea = SurfaceArea();
            return mSum * mMin * mArea;
        }

        AABB aabbs[2];

        int children[2] = {-1, -1};
        int parent = -1;
        int index = -1;
        int indexAsChild = -1;
        bool removed = false;

        std::vector<int> primitiveIndices;
    };
    struct Primitive {
        AABB aabb;
        int index = 0;
    };

    void Build(std::vector<Primitive> primitives);

    int BuildSAHBinned(Primitive* begin, Primitive* end, AABB aabb);
    int BuildSAHFull(Primitive* begin, Primitive* end, AABB aabb);
    void Optimize();

    template <typename F>
    bool Hit(const Ray& ray, F&& f) const;
    template <typename F, typename G>
    bool Intersect(Ray& ray, Interaction& it, F&& f, G&& g) const;
    AABB GetAABB() const {
        CHECK(nodes.size());
        return Union(nodes[rootIndex].aabbs[0], nodes[rootIndex].aabbs[1]);
    }

    int rootIndex = -1;
    std::vector<Node> nodes;
};

class BVH : public Accel {
  public:
    BVH(const Parameters&) {
    }

    void Initialize(const Scene* scene);
    bool Hit(Ray ray) const;
    bool Intersect(Ray& ray, Interaction& it) const;

    std::vector<BVHImpl> lbvh;
    BVHImpl tbvh;
    std::vector<int> indices;
    const Scene* scene;
};

}  // namespace pine

#endif  // PINE_IMPL_ACCEL_BVH_H