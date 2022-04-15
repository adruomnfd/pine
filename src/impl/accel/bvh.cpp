#include <impl/accel/bvh.h>
#include <core/scene.h>
#include <util/parameters.h>
#include <util/profiler.h>

#include <algorithm>
#include <queue>

namespace pine {

void BVHImpl::Build(pstd::vector<Primitive> primitives) {
    Profiler _("BuildBVH");
    LOG_PLAIN("[BVH]Building BVH");
    Timer timer;

    AABB aabb;
    for (auto& primitive : primitives)
        aabb.Extend(primitive.aabb);

    nodes.reserve(primitives.size());
    // BuildSAHBinned(primitives.data(), primitives.data() + primitives.size(), aabb);
    BuildSAHFull(&primitives[0], &primitives[0] + primitives.size(), aabb);
    rootIndex = (int)nodes.size() - 1;
    // Optimize();

    LOG_PLAIN(", & ms", timer.ElapsedMs());
    LOG_PLAIN(", & nodes(&.2 MB), & primitives(&.2 MB)\n", nodes.size(),
              nodes.size() * sizeof(nodes[0]) / 1000000.0, primitives.size(),
              primitives.size() * sizeof(primitives[0]) / 1000000.0);
}

int BVHImpl::BuildSAHBinned(Primitive* begin, Primitive* end, AABB aabb) {
    Node node;
    int numPrimitives = int(end - begin);
    CHECK_NE(numPrimitives, 0);

    auto MakeLeaf = [&]() {
        for (int i = 0; i < numPrimitives; i++)
            node.primitiveIndices.push_back(begin[i].index);
        for (Primitive* prim = begin; prim != end; prim++)
            node.aabbs[0].Extend(prim->aabb);
        node.aabbs[1] = node.aabbs[0];
        node.index = (int)nodes.size();
        nodes.push_back(node);
        return node.index;
    };
    if (numPrimitives == 1)
        return MakeLeaf();

    AABB aabbCentroid;
    for (int i = 0; i < numPrimitives; i++)
        aabbCentroid.Extend(begin[i].aabb.Centroid());
    float surfaceArea = aabb.SurfaceArea();

    struct Bucket {
        int count = 0;
        AABB aabb;
    };
    const int nBuckets = 16;

    float minCost = FloatMax;
    int bestAxis = -1;
    int splitBucket = -1;

    for (int axis = 0; axis < 3; axis++) {
        if (!aabbCentroid.IsValid(axis))
            continue;

        Bucket buckets[nBuckets];

        for (int i = 0; i < numPrimitives; i++) {
            int b =
                pstd::min(int(nBuckets * aabbCentroid.Offset(begin[i].aabb.Centroid(axis), axis)),
                          nBuckets - 1);
            buckets[b].count++;
            buckets[b].aabb.Extend(begin[i].aabb);
        }

        float cost[nBuckets - 1] = {};

        AABB bForward;
        int countForward = 0;
        for (int i = 0; i < nBuckets - 1; i++) {
            bForward.Extend(buckets[i].aabb);
            countForward += buckets[i].count;
            cost[i] += countForward * bForward.SurfaceArea();
        }

        AABB bBackward;
        int countBackward = 0;
        for (int i = nBuckets - 1; i >= 1; i--) {
            bBackward.Extend(buckets[i].aabb);
            countBackward += buckets[i].count;
            cost[i - 1] += countBackward * bBackward.SurfaceArea();
        }

        for (int i = 0; i < nBuckets - 1; i++) {
            cost[i] = 1.0f + cost[i] / surfaceArea;
        }

        float axisMinCost = FloatMax;
        int axisSplitBucket = -1;
        for (int i = 0; i < nBuckets - 1; i++) {
            if (cost[i] < axisMinCost) {
                axisMinCost = cost[i];
                axisSplitBucket = i;
            }
        }

        if (axisMinCost < minCost) {
            minCost = axisMinCost;
            bestAxis = axis;
            splitBucket = axisSplitBucket;
        }
    }

    float leafCost = numPrimitives;
    if (minCost > leafCost)
        return MakeLeaf();

    Primitive* pmid = std::partition(begin, end, [=](const Primitive& prim) {
        int b = nBuckets * aabbCentroid.Offset(prim.aabb.Centroid(bestAxis), bestAxis);
        if (b == nBuckets)
            b = nBuckets - 1;
        return b <= splitBucket;
    });

    CHECK(begin != pmid);
    CHECK(end != pmid);

    for (Primitive* prim = begin; prim != pmid; prim++)
        node.aabbs[0].Extend(prim->aabb);
    for (Primitive* prim = pmid; prim != end; prim++)
        node.aabbs[1].Extend(prim->aabb);
    node.children[0] = BuildSAHBinned(begin, pmid, node.aabbs[0]);
    node.children[1] = BuildSAHBinned(pmid, end, node.aabbs[1]);
    node.index = (int)nodes.size();
    nodes[node.children[0]].parent = node.index;
    nodes[node.children[1]].parent = node.index;
    nodes[node.children[0]].indexAsChild = 0;
    nodes[node.children[1]].indexAsChild = 1;

    nodes.push_back(node);
    return node.index;
}
int BVHImpl::BuildSAHFull(Primitive* begin, Primitive* end, AABB aabb) {
    Node node;
    int numPrimitives = int(end - begin);
    CHECK_NE(numPrimitives, 0);

    auto MakeLeaf = [&]() {
        for (int i = 0; i < numPrimitives; i++)
            node.primitiveIndices.push_back(begin[i].index);
        for (Primitive* prim = begin; prim != end; prim++)
            node.aabbs[0].Extend(prim->aabb);
        node.aabbs[1] = node.aabbs[0];
        node.index = (int)nodes.size();
        nodes.push_back(node);
        return node.index;
    };
    if (numPrimitives == 1)
        return MakeLeaf();

    AABB aabbCentroid;
    for (int i = 0; i < numPrimitives; i++)
        aabbCentroid.Extend(begin[i].aabb.Centroid());
    float surfaceArea = aabb.SurfaceArea();

    const int numSplits = numPrimitives - 1;
    enum class UseLowerOrUpperBound { Lower, Upper } useLowerOrUpperBound;
    float bestCost = FloatMax;
    int bestAxis = -1;
    int bestSplit = -1;
    pstd::vector<float> costs(numSplits);

    for (int axis = 0; axis < 3; axis++) {
        if (!aabbCentroid.IsValid(axis))
            continue;
        {  // AABB lower

            pstd::sort(begin, end, [&](const Primitive& l, const Primitive& r) {
                return l.aabb.lower[axis] < r.aabb.lower[axis];
            });

            AABB boundLeft;
            int countLeft = 0;
            for (int i = 0; i < numSplits; i++) {
                boundLeft.Extend(begin[i].aabb);
                countLeft++;
                costs[i] = boundLeft.SurfaceArea() * countLeft;
            }

            AABB boundRight;
            int countRight = 0;
            for (int i = numSplits - 1; i >= 0; i--) {
                boundRight.Extend(begin[i + 1].aabb);
                countRight++;
                costs[i] += boundRight.SurfaceArea() * countRight;
            }

            for (int i = 0; i < numSplits; i++) {
                float cost = 1.0f + costs[i] / surfaceArea;
                if (cost < bestCost) {
                    bestCost = cost;
                    bestAxis = axis;
                    bestSplit = i;
                    useLowerOrUpperBound = UseLowerOrUpperBound::Lower;
                }
            }
        }
        {  // AABB upper

            pstd::sort(begin, end, [&](const Primitive& l, const Primitive& r) {
                return l.aabb.upper[axis] < r.aabb.upper[axis];
            });

            AABB boundLeft;
            int countLeft = 0;
            for (int i = 0; i < numSplits; i++) {
                boundLeft.Extend(begin[i].aabb);
                countLeft++;
                costs[i] = boundLeft.SurfaceArea() * countLeft;
            }

            AABB boundRight;
            int countRight = 0;
            for (int i = numSplits - 1; i >= 0; i--) {
                boundRight.Extend(begin[i + 1].aabb);
                countRight++;
                costs[i] += boundRight.SurfaceArea() * countRight;
            }

            for (int i = 0; i < numSplits; i++) {
                float cost = 1.0f + costs[i] / surfaceArea;
                if (cost < bestCost) {
                    bestCost = cost;
                    bestAxis = axis;
                    bestSplit = i;
                    useLowerOrUpperBound = UseLowerOrUpperBound::Upper;
                }
            }
        }
    }

    Primitive* nthElement = begin + bestSplit + 1;
    if (useLowerOrUpperBound == UseLowerOrUpperBound::Lower)
        std::nth_element(begin, nthElement, end, [=](const Primitive& l, const Primitive& r) {
            return l.aabb.lower[bestAxis] < r.aabb.lower[bestAxis];
        });
    else if (useLowerOrUpperBound == UseLowerOrUpperBound::Upper)
        std::nth_element(begin, nthElement, end, [=](const Primitive& l, const Primitive& r) {
            return l.aabb.upper[bestAxis] < r.aabb.upper[bestAxis];
        });
    else
        CHECK(false);

    float leafCost = numPrimitives;

    if (bestCost > leafCost)
        return MakeLeaf();

    Primitive* pmid = begin + bestSplit + 1;
    CHECK(begin != pmid);
    CHECK(end != pmid);

    for (Primitive* prim = begin; prim != pmid; prim++)
        node.aabbs[0].Extend(prim->aabb);
    for (Primitive* prim = pmid; prim != end; prim++)
        node.aabbs[1].Extend(prim->aabb);
    node.children[0] = BuildSAHFull(begin, pmid, node.aabbs[0]);
    node.children[1] = BuildSAHFull(pmid, end, node.aabbs[1]);
    node.index = (int)nodes.size();
    nodes[node.children[0]].parent = node.index;
    nodes[node.children[1]].parent = node.index;
    nodes[node.children[0]].indexAsChild = 0;
    nodes[node.children[1]].indexAsChild = 1;

    nodes.push_back(node);
    return node.index;
}

struct Item {
    Item(int nodeIndex, float costInduced, float inverseCost)
        : nodeIndex(nodeIndex), costInduced(costInduced), inverseCost(inverseCost){};

    friend bool operator<(Item l, Item r) {
        return l.inverseCost < r.inverseCost;
    }

    int nodeIndex;
    float costInduced;
    float inverseCost;
};
struct Pair {
    friend bool operator<(Pair l, Pair r) {
        return l.inefficiency > r.inefficiency;
    }

    int index = -1;
    float inefficiency;
};
void BVHImpl::Optimize() {
    auto FindNodeForReinsertion = [&](int nodeIndex) {
        float eps = 1e-20f;
        float costBest = FloatMax;
        int nodeBest = -1;

        auto& L = nodes[nodeIndex];

        std::priority_queue<Item> queue;
        queue.push(Item(rootIndex, 0.0f, FloatMax));

        while (queue.size()) {
            Item item = queue.top();
            queue.pop();
            auto CiLX = item.costInduced;
            auto& X = nodes[item.nodeIndex];

            if (CiLX + L.SurfaceArea() >= costBest)
                break;

            float CdLX = Union(X.GetAABB(), L.GetAABB()).SurfaceArea();
            float CLX = CiLX + CdLX;
            if (CLX < costBest) {
                costBest = CLX;
                nodeBest = item.nodeIndex;
            }

            float Ci = CLX - X.SurfaceArea();

            if (Ci + L.SurfaceArea() < costBest) {
                if (X.primitiveIndices.size() == 0) {
                    queue.push(Item(X.children[0], Ci, 1.0f / (Ci + eps)));
                    queue.push(Item(X.children[1], Ci, 1.0f / (Ci + eps)));
                }
            }
        }

        return nodeBest;
    };

    RNG sampler;
    float startCost = nodes[rootIndex].ComputeCost(&nodes[0]) / 100000.0f;
    float lastCost = startCost;
    int numConvergedPasses = 0;
    for (int pass = 0; pass < 256; pass++) {
        if (pass % 5 == 1) {
            float cost = nodes[rootIndex].ComputeCost(&nodes[0]) / 100000.0f;
            LOG_SAMELINE("[BVH]SAH cost after & optimization passes: &", pass, cost);
            if (cost < lastCost * 0.99f) {
                numConvergedPasses = 0;
                lastCost = cost;
            } else {
                numConvergedPasses++;
                if (numConvergedPasses > 1) {
                    LOG_SAMELINE(
                        "[BVH]SAH cost converged to &.1(&.1%) after & optimization passes\n", cost,
                        100.0f * cost / startCost, pass);
                    break;
                }
            }
        }

        pstd::vector<pstd::pair<int, int>> unusedNodes;
        if (pass % 3 == 0) {
            pstd::vector<Pair> inefficiencies(nodes.size());

            for (int i = 0; i < (int)nodes.size(); i++)
                inefficiencies[i] = {i, nodes[i].Inefficiency()};
            std::partial_sort(inefficiencies.begin(), inefficiencies.begin() + nodes.size() / 200,
                              inefficiencies.end());

            for (int i = 0; i < (int)nodes.size() / 200; i++) {
                int nodeIndex = inefficiencies[i].index;
                if (nodes[nodeIndex].primitiveIndices.size() != 0 || nodes[nodeIndex].removed ||
                    nodes[nodeIndex].parent == -1 || nodes[nodes[nodeIndex].parent].removed ||
                    nodes[nodes[nodeIndex].parent].parent == -1 ||
                    nodes[nodes[nodes[nodeIndex].parent].parent].removed ||
                    nodes[nodes[nodes[nodeIndex].parent].parent].parent == -1)
                    continue;

                Node& node = nodes[nodeIndex];
                Node& parent = nodes[node.parent];
                Node& grandparent = nodes[parent.parent];
                node.removed = true;
                parent.removed = true;

                int secondChildOfParent = parent.children[1 - node.indexAsChild];
                grandparent.children[parent.indexAsChild] = secondChildOfParent;
                nodes[secondChildOfParent].indexAsChild = parent.indexAsChild;
                nodes[secondChildOfParent].parent = grandparent.index;
                grandparent.UpdateAABB(&nodes[0]);

                nodes[node.children[0]].removed = true;
                nodes[node.children[1]].removed = true;

                unusedNodes.push_back({node.children[0], nodeIndex});
                unusedNodes.push_back({node.children[1], node.parent});
            }
        } else {
            for (int i = 0; i < (int)nodes.size() / 100; i++) {
                int nodeIndex = -1;
                do {
                    nodeIndex = sampler.Uniform64u() % nodes.size();
                } while ((nodes[nodeIndex].primitiveIndices.size() != 0 ||
                          nodes[nodeIndex].removed || nodes[nodeIndex].parent == -1 ||
                          nodes[nodes[nodeIndex].parent].removed ||
                          nodes[nodes[nodeIndex].parent].parent == -1 ||
                          nodes[nodes[nodes[nodeIndex].parent].parent].removed ||
                          nodes[nodes[nodes[nodeIndex].parent].parent].parent == -1));

                Node& node = nodes[nodeIndex];
                Node& parent = nodes[node.parent];
                Node& grandparent = nodes[parent.parent];
                node.removed = true;
                parent.removed = true;

                int secondChildOfParent = parent.children[1 - node.indexAsChild];
                grandparent.children[parent.indexAsChild] = secondChildOfParent;
                nodes[secondChildOfParent].indexAsChild = parent.indexAsChild;
                nodes[secondChildOfParent].parent = grandparent.index;
                grandparent.UpdateAABB(&nodes[0]);

                nodes[node.children[0]].removed = true;
                nodes[node.children[1]].removed = true;

                unusedNodes.push_back({node.children[0], nodeIndex});
                unusedNodes.push_back({node.children[1], node.parent});
            }
        }

        pstd::sort(unusedNodes.begin(), unusedNodes.end(),
                   [&](pstd::pair<int, int> l, pstd::pair<int, int> r) {
                       return nodes[l.first].SurfaceArea() > nodes[r.first].SurfaceArea();
                   });

        for (pstd::pair<int, int> node : unusedNodes) {
            int L = node.first;
            int N = node.second;
            Node& x = nodes[FindNodeForReinsertion(L)];
            Node& n = nodes[N];
            Node& l = nodes[L];

            if (x.parent != -1)
                nodes[x.parent].children[x.indexAsChild] = n.index;
            else
                rootIndex = n.index;
            n.parent = x.parent;
            n.indexAsChild = x.indexAsChild;
            n.removed = false;

            n.children[0] = x.index;
            n.children[1] = l.index;
            x.parent = n.index;
            l.parent = n.index;

            x.indexAsChild = 0;
            l.indexAsChild = 1;
            l.removed = false;
            n.UpdateAABB(&nodes[0]);
        }
    }
}

template <typename F>
bool BVHImpl::Hit(const Ray& ray, F&& f) const {
    RayOctant rayOctant = RayOctant(ray);
    const Node* PINE_RESTRICT nodes = this->nodes.data();

    int stack[32];
    int ptr = 0;
    int next = rootIndex;

    if (PINE_UNLIKELY(nodes[next].primitiveIndices.size())) {
        for (int index : nodes[next].primitiveIndices)
            if (f(ray, index))
                return true;
    } else {
        while (true) {
            const Node& node = nodes[next];

            int leftChildIndex = -1, rightChildIndex = -1;
            float t0 = ray.tmax, t1 = ray.tmax;
            if (node.aabbs[0].Hit(rayOctant, ray.tmin, &t0)) {
                const Node& leftChild = nodes[node.children[0]];
                if (PINE_LIKELY(!leftChild.primitiveIndices.size())) {
                    leftChildIndex = node.children[0];
                } else {
                    for (int index : leftChild.primitiveIndices)
                        if (f(ray, index))
                            return true;
                }
            }
            if (node.aabbs[1].Hit(rayOctant, ray.tmin, &t1)) {
                const Node& rightChild = nodes[node.children[1]];
                if (PINE_LIKELY(!rightChild.primitiveIndices.size())) {
                    rightChildIndex = node.children[1];

                } else {
                    for (int index : rightChild.primitiveIndices)
                        if (f(ray, index))
                            return true;
                }
            }

            if (leftChildIndex != -1) {
                if (rightChildIndex != -1) {
                    if (t0 > t1) {
                        stack[ptr++] = leftChildIndex;
                        next = rightChildIndex;
                    } else {
                        stack[ptr++] = rightChildIndex;
                        next = leftChildIndex;
                    }
                } else {
                    next = leftChildIndex;
                }
            } else if (rightChildIndex != -1) {
                next = rightChildIndex;
            } else {
                if (PINE_UNLIKELY(ptr == 0))
                    break;
                next = stack[--ptr];
            }
        }
    }

    return false;
}

template <typename F, typename G>
bool BVHImpl::Intersect(Ray& ray, Interaction& it, F&& f, G&& g) const {
    RayOctant rayOctant = RayOctant(ray);
    const Node* PINE_RESTRICT nodes = this->nodes.data();

    bool hit = false;
    int closestIndex = -1;
    int stack[32];
    int ptr = 0;
    int next = rootIndex;

    if (PINE_UNLIKELY(nodes[next].primitiveIndices.size())) {
        for (int index : nodes[next].primitiveIndices)
            if (f(ray, it, index)) {
                hit = true;
                closestIndex = index;
            }
    } else {
        while (true) {
            it.bvh += 2.0f;
            const Node& node = nodes[next];

            int leftChildIndex = -1, rightChildIndex = -1;
            float t0 = ray.tmax, t1 = ray.tmax;
            if (node.aabbs[0].Hit(rayOctant, ray.tmin, &t0)) {
                const Node& leftChild = nodes[node.children[0]];
                if (PINE_LIKELY(!leftChild.primitiveIndices.size())) {
                    leftChildIndex = node.children[0];
                } else {
                    for (int index : leftChild.primitiveIndices)
                        if (f(ray, it, index)) {
                            hit = true;
                            closestIndex = index;
                        }
                }
            }
            if (node.aabbs[1].Hit(rayOctant, ray.tmin, &t1)) {
                const Node& rightChild = nodes[node.children[1]];
                if (PINE_LIKELY(!rightChild.primitiveIndices.size())) {
                    rightChildIndex = node.children[1];

                } else {
                    for (int index : rightChild.primitiveIndices)
                        if (f(ray, it, index)) {
                            hit = true;
                            closestIndex = index;
                        }
                }
            }

            if (leftChildIndex != -1) {
                if (rightChildIndex != -1) {
                    if (t0 > t1) {
                        stack[ptr++] = leftChildIndex;
                        next = rightChildIndex;
                    } else {
                        stack[ptr++] = rightChildIndex;
                        next = leftChildIndex;
                    }
                } else {
                    next = leftChildIndex;
                }
            } else if (rightChildIndex != -1) {
                next = rightChildIndex;
            } else {
                if (PINE_UNLIKELY(ptr == 0))
                    break;
                next = stack[--ptr];
            }
        }
    }

    if (closestIndex != -1)
        g(it, closestIndex);

    return hit;
}

void BVH::Initialize(const Scene* scene) {
    this->scene = scene;
    if (scene->shapes.size() == 0)
        return;

    for (int i = 0; i < (int)scene->shapes.size(); i++) {
        if (scene->shapes[i].Is<TriangleMesh>()) {
            pstd::vector<BVHImpl::Primitive> primitives;
            for (auto& t : scene->shapes[i].Be<TriangleMesh>().ToTriangles()) {
                BVHImpl::Primitive primitive;
                primitive.aabb = t.GetAABB();
                primitive.index = (int)primitives.size();
                primitives.push_back(primitive);
            }
            BVHImpl bvh;
            bvh.Build(pstd::move(primitives));
            lbvh.push_back(pstd::move(bvh));
            indices.push_back(i);
        }
    }

    pstd::vector<BVHImpl::Primitive> primitives;
    for (auto& s : lbvh) {
        BVHImpl::Primitive primitive;
        primitive.aabb = s.GetAABB();
        primitive.index = (int)primitives.size();
        primitives.push_back(primitive);
    }
    for (int i = 0; i < (int)scene->shapes.size(); i++) {
        if (!scene->shapes[i].Is<TriangleMesh>()) {
            BVHImpl::Primitive primitive;
            primitive.aabb = scene->shapes[i].GetAABB();
            primitive.index = (int)primitives.size();
            primitives.push_back(primitive);
            indices.push_back(i);
        }
    }
    tbvh.Build(primitives);
}

bool BVH::Hit(Ray ray) const {
    if (scene->shapes.size() == 0)
        return false;

    return tbvh.Hit(ray, [&](const Ray& ray, int lbvhIndex) {
        auto& shape = scene->shapes[indices[lbvhIndex]];

        if (lbvhIndex < (int)lbvh.size()) {
            return lbvh[lbvhIndex].Hit(ray, [&](const Ray& ray, int index) {
                return shape.Be<TriangleMesh>().GetTriangle(index).Hit(ray);
            });
        } else {
            return shape.Hit(ray);
        }
    });
}

bool BVH::Intersect(Ray& ray, Interaction& it) const {
    if (scene->shapes.size() == 0)
        return false;

    int triangleIndex = -1;
    return tbvh.Intersect(
        ray, it,
        [&](Ray& ray, Interaction& it, int lbvhIndex) {
            auto& shape = scene->shapes[indices[lbvhIndex]];

            if (lbvhIndex < (int)lbvh.size()) {
                return lbvh[lbvhIndex].Intersect(
                    ray, it,
                    [&](Ray& ray, Interaction& it, int index) {
                        return shape.Be<TriangleMesh>().GetTriangle(index).Intersect(ray, it);
                    },
                    [&](Interaction&, int tIndex) { triangleIndex = tIndex; });
            } else {
                return shape.Intersect(ray, it);
            }
        },
        [&](Interaction&, int lbvhIndex) {
            auto& shape = scene->shapes[indices[lbvhIndex]];
            if (lbvhIndex < (int)lbvh.size()) {
                auto& mesh = shape.Be<TriangleMesh>();
                Triangle tri = mesh.GetTriangle(triangleIndex);
                it.p = tri.InterpolatePosition(it.uv);
                it.n = Normalize(Cross(tri.v0 - tri.v1, tri.v0 - tri.v2));
                tri.ComputeDpDuv(it.dpdu, it.dpdv);
            }

            it.shape = &shape;
            it.material = shape.material.get();
            if (shape.mediumInterface.IsMediumTransition()) {
                it.mediumInterface.inside = shape.mediumInterface.inside.get();
                it.mediumInterface.outside = shape.mediumInterface.outside.get();
            } else {
                it.mediumInterface = MediumInterface<const Medium*>(ray.medium);
            }
        });
}

}  // namespace pine