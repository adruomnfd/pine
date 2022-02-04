#include <impl/accel/bvh.h>
#include <util/parameters.h>
#include <util/profiler.h>

#include <algorithm>
#include <queue>

namespace pine {

void BVH::Initialize(const TriangleMesh* mesh) {
    Profiler _("BVHBuild");
    LOG_VERBOSE("[BVH]Building BVH");
    Timer timer;

    AABB aabb;
    std::vector<Primitive> primitives;
    for (int i = 0; i < mesh->GetNumTriangles(); i++) {
        Primitive prim;
        prim.triangle = mesh->GetTriangle(i);
        prim.aabb = prim.triangle.GetAABB();
        aabb.Extend(prim.aabb);
        primitives.push_back(prim);
    }

    nodes.reserve(primitives.size());
    // BuildSAHBinned(primitives.data(), primitives.data() + primitives.size(), aabb);
    // BuildSAHFull(primitives.data(), primitives.data() + primitives.size(), aabb);
    BuildSpatialSplit(primitives.data(), primitives.data() + primitives.size(), aabb,
                      aabb.SurfaceArea());
    rootIndex = (int)nodes.size() - 1;
    // Optimize();

    LOG_VERBOSE("[BVH]BVH built in & ms", timer.ElapsedMs());
    LOG_VERBOSE("[BVH]Resulting BVH has & nodes(&.2 MB), & primitives(&.2 MB)", nodes.size(),
                nodes.size() * sizeof(nodes[0]) / 1000000.0, primitives.size(),
                primitives.size() * sizeof(primitives[0]) / 1000000.0);
}

int BVH::BuildSAHBinned(Primitive* begin, Primitive* end, AABB aabb) {
    Node node;
    int numPrimitives = int(end - begin);
    CHECK_NE(numPrimitives, 0);

    auto MakeLeaf = [&]() {
        node.numPrimitives = numPrimitives;
        node.triangles = new Triangle[numPrimitives];
        for (int i = 0; i < numPrimitives; i++)
            node.triangles[i] = begin[i].triangle;
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
                std::min(int(nBuckets * aabbCentroid.Offset(begin[i].aabb.Centroid(axis), axis)),
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

int BVH::BuildSAHFull(Primitive* begin, Primitive* end, AABB aabb) {
    Node node;
    int numPrimitives = int(end - begin);
    CHECK_NE(numPrimitives, 0);

    auto MakeLeaf = [&]() {
        node.numPrimitives = numPrimitives;
        node.triangles = new Triangle[numPrimitives];
        for (int i = 0; i < numPrimitives; i++)
            node.triangles[i] = begin[i].triangle;
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
    std::vector<float> costs(numSplits);

    for (int axis = 0; axis < 3; axis++) {
        if (!aabbCentroid.IsValid(axis))
            continue;
        {  // AABB lower

            std::sort(begin, end, [&](const Primitive& l, const Primitive& r) {
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

            std::sort(begin, end, [&](const Primitive& l, const Primitive& r) {
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

int BVH::BuildSpatialSplit(Primitive* begin, Primitive* end, AABB aabb, float SAroot, int depth) {
    Node node;
    int numPrimitives = int(end - begin);
    CHECK_NE(numPrimitives, 0);

    auto MakeLeaf = [&]() {
        node.numPrimitives = numPrimitives;
        node.triangles = new Triangle[numPrimitives];
        for (int i = 0; i < numPrimitives; i++)
            node.triangles[i] = begin[i].triangle;
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

    struct Bin {
        int entries = 0;
        int exits = 0;
        AABB aabb;
    };
    const int nBins = 64;
    const int nSplits = nBins - 1;
    const vec3 binSize = aabb.Diagonal() / nBins;

    int spatialSplitBestAxis = -1;
    int spatialSplitIndex = -1;
    float spatialMinCost = FloatMax;

    int objectSplitIndex = -1;
    float objectMinCost = FloatMax;
    float SAintersection = 0.0f;

    {  // Object splitting
        const int numObjectSplits = numPrimitives - 1;
        enum class UseLowerOrUpperBound { Lower, Upper } useLowerOrUpperBound;
        std::vector<float> costs(numObjectSplits);

        int bestAxis = -1;
        for (int axis = 0; axis < 3; axis++) {
            if (!aabbCentroid.IsValid(axis))
                continue;
            {  // AABB lower

                std::sort(begin, end, [&](const Primitive& l, const Primitive& r) {
                    return l.aabb.lower[axis] < r.aabb.lower[axis];
                });

                AABB boundLeft;
                int countLeft = 0;
                for (int i = 0; i < numObjectSplits; i++) {
                    boundLeft.Extend(begin[i].aabb);
                    countLeft++;
                    costs[i] = boundLeft.SurfaceArea() * countLeft;
                }

                AABB boundRight;
                int countRight = 0;
                for (int i = numObjectSplits - 1; i >= 0; i--) {
                    boundRight.Extend(begin[i + 1].aabb);
                    countRight++;
                    costs[i] += boundRight.SurfaceArea() * countRight;
                }

                for (int i = 0; i < numObjectSplits; i++) {
                    float cost = 1.0f + costs[i] / surfaceArea;
                    if (cost < objectMinCost) {
                        objectMinCost = cost;
                        bestAxis = axis;
                        objectSplitIndex = i;
                        useLowerOrUpperBound = UseLowerOrUpperBound::Lower;
                    }
                }
            }
            {  // AABB upper
                std::vector<float> costs(numObjectSplits);

                std::sort(begin, end, [&](const Primitive& l, const Primitive& r) {
                    return l.aabb.upper[axis] < r.aabb.upper[axis];
                });

                AABB boundLeft;
                int countLeft = 0;
                for (int i = 0; i < numObjectSplits; i++) {
                    boundLeft.Extend(begin[i].aabb);
                    countLeft++;
                    costs[i] = boundLeft.SurfaceArea() * countLeft;
                }

                AABB boundRight;
                int countRight = 0;
                for (int i = numObjectSplits - 1; i >= 0; i--) {
                    boundRight.Extend(begin[i + 1].aabb);
                    countRight++;
                    costs[i] += boundRight.SurfaceArea() * countRight;
                }

                for (int i = 0; i < numObjectSplits; i++) {
                    float cost = 1.0f + costs[i] / surfaceArea;
                    if (cost < objectMinCost) {
                        objectMinCost = cost;
                        bestAxis = axis;
                        objectSplitIndex = i;
                        useLowerOrUpperBound = UseLowerOrUpperBound::Upper;
                    }
                }
            }
        }

        Primitive* nthElement = begin + objectSplitIndex + 1;
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

        AABB boundLeft, boundRight;
        for (Primitive* prim = begin; prim != nthElement; prim++)
            boundLeft.Extend(prim->aabb);
        for (Primitive* prim = nthElement; prim != end; prim++)
            boundRight.Extend(prim->aabb);
        SAintersection = Intersection(boundLeft, boundRight).SurfaceArea();
    }

    {  // Spatial splitting
        int axis = aabbCentroid.MaxDim();
        // for (int axis = 0; axis < 3; axis++) {
        Bin spatialBins[nBins];
        for (int i = 0; i < numPrimitives; i++) {
            struct Line {
                AABB GetAABB(float left, float right, int dim) {
                    AABB aabb;
                    if (p1[dim] - p0[dim] == 0.0f)
                        return aabb;
                    float t0 = Clamp((left - p0[dim]) / (p1[dim] - p0[dim]), 0.0f, 1.0f);
                    float t1 = Clamp((right - p0[dim]) / (p1[dim] - p0[dim]), 0.0f, 1.0f);
                    aabb.Extend(p0 * (1.0f - t0) + p1 * t0);
                    aabb.Extend(p0 * (1.0f - t1) + p1 * t1);
                    return aabb;
                }
                vec3 p0;
                vec3 p1;
            };
            int triangleLeftBin = std::floor(aabb.Offset(begin[i].aabb.lower[axis], axis) * nBins);
            int triangleRightBin = std::floor(aabb.Offset(begin[i].aabb.upper[axis], axis) * nBins);
            CHECK_GE(begin[i].aabb.upper[axis], begin[i].aabb.lower[axis]);
            CHECK_GE(triangleRightBin, triangleLeftBin);
            triangleLeftBin = Clamp(triangleLeftBin, 0, nBins - 1);
            triangleRightBin = Clamp(triangleRightBin, 0, nBins - 1);
            spatialBins[triangleLeftBin].entries++;
            spatialBins[triangleRightBin].exits++;

            for (int j = 0; j < 3; j++) {
                Line line = {begin[i].triangle.Vertex(j), begin[i].triangle.Vertex((j + 1) % 3)};
                int lineLeftBin = std::floor(aabb.Offset(line.p0[axis], axis) * nBins);
                int lineRightBin = std::floor(aabb.Offset(line.p1[axis], axis) * nBins);
                lineLeftBin = Clamp(lineLeftBin, triangleLeftBin, triangleRightBin);
                lineRightBin = Clamp(lineRightBin, triangleLeftBin, triangleRightBin);
                lineLeftBin = Clamp(lineLeftBin, 0, nBins - 1);
                lineRightBin = Clamp(lineRightBin, 0, nBins - 1);

                for (int k = lineLeftBin; k <= lineRightBin; k++) {
                    float posLeft = aabb.lower[axis] + binSize[axis] * k;
                    float posRight = aabb.lower[axis] + binSize[axis] * (k + 1);
                    AABB aabb = line.GetAABB(posLeft, posRight, axis);
                    aabb.lower = Max(aabb.lower, begin[i].aabb.lower);
                    aabb.upper = Min(aabb.upper, begin[i].aabb.upper);
                    spatialBins[k].aabb.Extend(aabb);
                }
            }
        }
        float costs[nSplits] = {};

        AABB bForward;
        int countForward = 0;
        for (int i = 0; i < nSplits; i++) {
            bForward.Extend(spatialBins[i].aabb);
            countForward += spatialBins[i].entries;
            costs[i] = countForward * bForward.SurfaceArea();
        }
        AABB bBackward;
        int countBackward = 0;
        for (int i = nSplits - 1; i >= 0; i--) {
            bBackward.Extend(spatialBins[i + 1].aabb);
            countBackward += spatialBins[i + 1].exits;
            costs[i] += countBackward * bBackward.SurfaceArea();
        }

        for (int i = 0; i < nSplits; i++) {
            float cost = 1.0f + costs[i] / surfaceArea;
            if (cost < spatialMinCost) {
                spatialMinCost = cost;
                spatialSplitIndex = i;
                spatialSplitBestAxis = axis;
            }
        }
        // }
    }

    float leafCost = numPrimitives;
    float lambda = SAintersection / SAroot;
    // LOG("&-6.2 &-6.2 &-6.2 &-6.2 &-6.6 & &-6.2 &-6.2 & &6.2 &", numPrimitives, depth,
    // objectMinCost,
    //     spatialMinCost, lambda, spatialMinCost < objectMinCost && lambda > 1e-5f, aabb.lower,
    //     aabb.upper, spatialSplitBestAxis, SAintersection, SAroot);

    if (std::min(spatialMinCost, objectMinCost) > leafCost)
        return MakeLeaf();

    float spatialSplitPos =
        aabb.lower[spatialSplitBestAxis] + (spatialSplitIndex + 1) * binSize[spatialSplitBestAxis];
    std::vector<Primitive> primLeft;
    std::vector<Primitive> primRight;
    for (const Primitive* prim = begin; prim != end; prim++) {
        if (prim->aabb.lower[spatialSplitBestAxis] < spatialSplitPos)
            primLeft.push_back(*prim);
        if (prim->aabb.upper[spatialSplitBestAxis] > spatialSplitPos)
            primRight.push_back(*prim);
    }

    bool vaildSplitting = primLeft.size() && primRight.size() &&
                          (int)primLeft.size() != numPrimitives &&
                          (int)primRight.size() != numPrimitives;

    if (spatialMinCost < objectMinCost && lambda > 1e-5f && vaildSplitting) {
        CHECK_NE(primLeft.size(), 0);
        CHECK_NE(primRight.size(), 0);

        for (auto& p : primLeft) {
            p.aabb.upper[spatialSplitBestAxis] =
                std::min(p.aabb.upper[spatialSplitBestAxis], spatialSplitPos);
        }
        for (auto& p : primRight) {
            p.aabb.lower[spatialSplitBestAxis] =
                std::max(p.aabb.lower[spatialSplitBestAxis], spatialSplitPos);
        }

        for (auto& p : primLeft)
            node.aabbs[0].Extend(p.aabb);
        for (auto& p : primRight)
            node.aabbs[1].Extend(p.aabb);

        node.children[0] = BuildSpatialSplit(primLeft.data(), primLeft.data() + primLeft.size(),
                                             node.aabbs[0], SAroot, depth + 1);
        node.children[1] = BuildSpatialSplit(primRight.data(), primRight.data() + primRight.size(),
                                             node.aabbs[1], SAroot, depth + 1);
    } else {
        Primitive* pmid = begin + objectSplitIndex + 1;

        CHECK(begin != pmid);
        CHECK(end != pmid);

        for (Primitive* prim = begin; prim != pmid; prim++)
            node.aabbs[0].Extend(prim->aabb);
        for (Primitive* prim = pmid; prim != end; prim++)
            node.aabbs[1].Extend(prim->aabb);
        node.children[0] = BuildSpatialSplit(begin, pmid, node.aabbs[0], SAroot, depth + 1);
        node.children[1] = BuildSpatialSplit(pmid, end, node.aabbs[1], SAroot, depth + 1);
    }

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

void BVH::Optimize() {
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
                if (X.triangles == nullptr) {
                    queue.push(Item(X.children[0], Ci, 1.0f / (Ci + eps)));
                    queue.push(Item(X.children[1], Ci, 1.0f / (Ci + eps)));
                }
            }
        }

        return nodeBest;
    };

    RNG rng;
    float startCost = nodes[rootIndex].ComputeCost(nodes.data()) / 100000.0f;
    float lastCost = startCost;
    int numConvergedPasses = 0;
    for (int pass = 0; pass < 256; pass++) {
        if (pass % 5 == 1) {
            float cost = nodes[rootIndex].ComputeCost(nodes.data()) / 100000.0f;
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

        std::vector<std::pair<int, int>> unusedNodes;
        if (pass % 3 == 0) {
            std::vector<Pair> inefficiencies(nodes.size());

            for (int i = 0; i < (int)nodes.size(); i++)
                inefficiencies[i] = {i, nodes[i].Inefficiency()};
            std::partial_sort(inefficiencies.begin(), inefficiencies.begin() + nodes.size() / 200,
                              inefficiencies.end());

            for (int i = 0; i < (int)nodes.size() / 200; i++) {
                int nodeIndex = inefficiencies[i].index;
                if (nodes[nodeIndex].triangles || nodes[nodeIndex].removed ||
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
                grandparent.UpdateAABB(nodes.data());

                nodes[node.children[0]].removed = true;
                nodes[node.children[1]].removed = true;

                unusedNodes.push_back({node.children[0], nodeIndex});
                unusedNodes.push_back({node.children[1], node.parent});
            }
        } else {
            for (int i = 0; i < (int)nodes.size() / 100; i++) {
                int nodeIndex = -1;
                do {
                    nodeIndex = rng.Uniform64u() % nodes.size();
                } while ((nodes[nodeIndex].triangles || nodes[nodeIndex].removed ||
                          nodes[nodeIndex].parent == -1 || nodes[nodes[nodeIndex].parent].removed ||
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
                grandparent.UpdateAABB(nodes.data());

                nodes[node.children[0]].removed = true;
                nodes[node.children[1]].removed = true;

                unusedNodes.push_back({node.children[0], nodeIndex});
                unusedNodes.push_back({node.children[1], node.parent});
            }
        }

        std::sort(unusedNodes.begin(), unusedNodes.end(),
                  [&](std::pair<int, int> l, std::pair<int, int> r) {
                      return nodes[l.first].SurfaceArea() > nodes[r.first].SurfaceArea();
                  });

        for (std::pair<int, int> node : unusedNodes) {
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
            n.UpdateAABB(nodes.data());
        }
    }
}

bool BVH::Hit(Ray) const {
    SampledProfiler _(ProfilePhase::IntersectShadow);
    return false;
}

bool BVH::Intersect(Ray& ray, Interaction& it) const {
    SampledProfiler _(ProfilePhase::IntersectClosest);

    const vec3 invDir = SafeRcp(ray.d);
    const vec3 negOrgDivDir = -ray.o * invDir;
    int octantx3[3] = {(int)std::signbit(ray.d[0]) * 3, (int)std::signbit(ray.d[1]) * 3,
                       (int)std::signbit(ray.d[2]) * 3};

    bool hit = false;
    const Triangle* closestTriangle = nullptr;
    const Node* PINE_RESTRICT nodes = this->nodes.data();

    int stack[32];
    int ptr = 0;
    int next = rootIndex;

    if (PINE_UNLIKELY(nodes[next].numPrimitives)) {
        for (int i = 0; i < nodes[next].numPrimitives; i++)
            if (nodes[next].triangles[i].Intersect(ray, it)) {
                hit = true;
                closestTriangle = nodes[next].triangles + i;
            }
    } else {
        while (true) {
            it.bvh += 2.0f;
            const Node& node = nodes[next];

            int leftChildIndex = -1, rightChildIndex = -1;
            float t0 = ray.tmax, t1 = ray.tmax;
            if (node.aabbs[0].Hit(octantx3, negOrgDivDir, invDir, ray.tmin, &t0)) {
                const Node& leftChild = nodes[node.children[0]];
                if (PINE_LIKELY(!leftChild.numPrimitives)) {
                    leftChildIndex = node.children[0];
                } else {
                    for (int i = 0; i < leftChild.numPrimitives; i++)
                        if (leftChild.triangles[i].Intersect(ray, it)) {
                            hit = true;
                            closestTriangle = leftChild.triangles + i;
                        }
                }
            }
            if (node.aabbs[1].Hit(octantx3, negOrgDivDir, invDir, ray.tmin, &t1)) {
                const Node& rightChild = nodes[node.children[1]];
                if (PINE_LIKELY(!rightChild.numPrimitives)) {
                    rightChildIndex = node.children[1];

                } else {
                    for (int i = 0; i < rightChild.numPrimitives; i++)
                        if (rightChild.triangles[i].Intersect(ray, it)) {
                            hit = true;
                            closestTriangle = rightChild.triangles + i;
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

    if (hit) {
        it.p = closestTriangle->InterpolatePosition(it.uv);
        it.n = closestTriangle->Normal();
    }

    return hit;
}

}  // namespace pine