// #ifndef PINE_IMPL_ACCEL_CWBVH_H
// #define PINE_IMPL_ACCEL_CWBVH_H

// #include <core/accel.h>

// namespace pine {

// class CWBVH : public Accel {
//   public:
//     struct alignas(16) CompactTriangle {
//         vec3 v0;
//         vec3 v1;
//         vec3 v2;
//         int orginalIndex = -1;
//     };

//     struct Node8Compressed {
//         void SetParentAABB(AABB B);
//         void SetChildAABB(int index, AABB b);

//         void SetChildAsTriangle(int index, int triangleNumOffset, int numTriangles) {
//             meta[index] = (((1u << numTriangles) - 1) << 5) + triangleNumOffset;
//             CHECK_EQ(GetNumTriangles(index), numTriangles);
//         };
//         void SetChildAsNode(int index) {
//             meta[index] = (0b001u << 5) + 24 + index;
//             imask |= 1u << index;
//         }
//         uint32_t GetChildIndex(int index) const {
//             static constexpr auto popc = [](uint32_t v) {
//                 int b = 0;
//                 while (v) {
//                     if (1u & v) {
//                         b++;
//                     }
//                     v >>= 1;
//                 }
//                 return b;
//             };
//             return childBaseIndex + popc(imask & ~(-1 << index));
//         }
//         uint32_t GetTriangleIndex(int index) const {
//             return triangleBaseIndex + (meta[index] & 0b00011111);
//         }
//         int GetNumTriangles(int index) const {
//             int n = meta[index] >> 5;
//             if (n == 1)
//                 return 1;
//             else
//                 return (n == 3) ? 2 : 3;
//         }
//         bool IsInternalNode(int index) const {
//             return (1 & (imask >> index));
//         }
//         bool IsEmptyNode(int index) const {
//             return meta[index] == 0;
//         }

//         vec3 p;
//         int8_t e[3];
//         // uint8_t qlo[3][8];
//         // uint8_t qhi[3][8];
//         uint8_t qlohi[3][2][8];
//         uint8_t imask = 0;
//         uint32_t childBaseIndex = uint32_t(-1);
//         uint32_t triangleBaseIndex = uint32_t(-1);
//         union {
//             uint8_t meta[8] = {};
//             uint32_t meta4[2];
//             uint64_t meta8;
//         };
//     };

//     struct Node8 {
//         void Compress(std::vector<Node8Compressed>& nodes, std::vector<CompactTriangle>&
//         triangles,
//                       uint32_t index);
//         void Destory();

//         AABB parentAABB;
//         AABB aabb[8];
//         Node8* children[8] = {};
//         std::vector<CompactTriangle> triangles[8] = {};
//     };

//     struct Node2 {
//         float SurfaceArea() const {
//             return aabb.SurfaceArea();
//         }
//         void UpdateAABB(Node2* nodes) {
//             aabb = Union(nodes[children[0]].aabb, nodes[children[1]].aabb);
//             if (parent != -1)
//                 nodes[parent].UpdateAABB(nodes);
//         }
//         float ComputeSAHCost(Node2* nodes) {
//             if (!isInternalNode)
//                 return SurfaceArea();
//             return nodes[children[0]].ComputeSAHCost(nodes) +
//                    nodes[children[1]].ComputeSAHCost(nodes) + SurfaceArea();
//         }
//         float Inefficiency(Node2* nodes) const {
//             if (!isInternalNode)
//                 return SurfaceArea();
//             float mSum =
//                 SurfaceArea() /
//                 (2 * (nodes[children[0]].SurfaceArea() + nodes[children[1]].SurfaceArea()));
//             float mMin = SurfaceArea() / std::min(nodes[children[0]].SurfaceArea(),
//                                                   nodes[children[1]].SurfaceArea());
//             float mArea = SurfaceArea();
//             return mSum * mMin * mArea;
//         }

//         void ComputeCost(Node2* nodes, int depth = 0);
//         float CostNode(Node2* nodes, int i);
//         float CostLeaf(Node2* nodes) const;
//         float CostInternal(Node2* nodes);
//         float CostDistribute(Node2* nodes, int j);
//         int NumPrimitives(Node2* nodes) const;
//         void CollapseToNode8(Node2* nodes2, Node8* node8, int numRoots, int index);

//         AABB aabb;
//         CompactTriangle triangle;

//         bool isInternalNode = false;
//         int children[2] = {-1, -1};
//         int parent = -1;
//         int index = -1;
//         int indexAsChild = -1;
//         bool removed = false;

//         float cost[8] = {};
//         enum class NodeType {
//             Leaf,
//             Internal,
//         } createLeafOrInternal;
//         int distributeLeft[9] = {};
//         int distributeRight[9] = {};
//     };

//     struct Cluster {
//         int Flatten(std::vector<Node2>& nodes) const;
//         void Destory();

//         AABB aabb;
//         CompactTriangle triangle;
//         Cluster* children = nullptr;

//         uint32_t mortonCode = 0;
//         bool invalid = false;
//     };

//     CWBVH(const Parameters&) {
//     }
//     void Initialize(const TriangleMesh* mesh) override;

//     void BuildLocalOrderedBinaryBVH(const TriangleMesh* mesh);
//     void BuildBinnedBVH(const TriangleMesh* mesh);
//     void BuildSweepSahBVH(const TriangleMesh* mesh);

//     void ReinsertionOptimization();

//     bool Hit(Ray ray) const override;
//     bool Intersect(Ray& ray, Interaction& it) const override;

//     std::vector<Cluster> clusters;
//     std::vector<Node2> nodes2;
//     int node2Root = -1;
//     Node8* wideTreeRoot = nullptr;
//     std::vector<Node8Compressed> nodes;
//     std::vector<CompactTriangle> triangles;
//     const TriangleMesh* mesh;

//     static constexpr float kCostPrimitive = 0.3f;
//     static constexpr float kCostNode = 1.0f;
//     static constexpr int kNodeMaxNumPrimitives = 3;
//     static constexpr int r = 14;
// };

// }  // namespace pine

// #endif  // PINE_IMPL_ACCEL_CWBVH_H