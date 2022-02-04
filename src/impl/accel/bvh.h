#ifndef PINE_IMPL_ACCEL_BVH_H
#define PINE_IMPL_ACCEL_BVH_H

#include <core/accel.h>

#include <memory>
#include <vector>

namespace pine {

class BVH : public Accel {
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
            if (triangles)
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

        int numPrimitives = 0;
        Triangle* triangles = nullptr;
    };
    struct Primitive {
        AABB aabb;
        Triangle triangle;
    };

    BVH(const Parameters&) {
    }
    ~BVH() {
        for (Node& node : nodes)
            if (node.triangles)
                delete[] node.triangles;
    }
    PINE_DELETE_COPY_MOVE(BVH)

    void Initialize(const TriangleMesh* mesh) override;
    int BuildSAHBinned(Primitive* begin, Primitive* end, AABB aabb);
    int BuildSAHFull(Primitive* begin, Primitive* end, AABB aabb);
    int BuildSpatialSplit(Primitive* begin, Primitive* end, AABB aabb, float SAroot, int depth = 0);
    void Optimize();

    bool Hit(Ray ray) const override;
    bool Intersect(Ray& ray, Interaction& it) const override;

    int rootIndex = -1;
    std::vector<Node> nodes;
};

}  // namespace pine

#endif  // PINE_IMPL_ACCEL_BVH_H