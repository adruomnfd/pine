// #include <impl/accel/cwbvh.h>
// #include <util/parallel.h>

// #include <algorithm>
// #include <queue>

// namespace pine {

// void CWBVH::Initialize(const TriangleMesh* mesh) {
//     Profiler _("BuildBVH");
//     Timer timer;

//     this->mesh = mesh;

//     if (0) {
//         LOG_VERBOSE_SAMELINE("[CWBVH]Building local ordered BVH");
//         BuildLocalOrderedBinaryBVH(mesh);
//         LOG_VERBOSE_SAMELINE("[CWBVH]Building local ordered BVH (&.1 ms)\n", timer.Reset());

//         LOG_VERBOSE_SAMELINE("[CWBVH]Flattening BVH");
//         nodes2.reserve(clusters.size());
//         clusters[0].Flatten(nodes2);
//         node2Root = 0;
//         clusters[0].Destory();
//         LOG_VERBOSE_SAMELINE("[CWBVH]Flattening BVH             (&.1 ms)\n", timer.Reset());
//     } else {
//         LOG_VERBOSE_SAMELINE("[CWBVH]Building binary BVH");
//         nodes2.reserve(mesh->GetNumTriangles());
//         BuildBinnedBVH(mesh);
//         // BuildSweepSahBVH(mesh);
//         node2Root = 0;
//         LOG_VERBOSE_SAMELINE("[CWBVH]Building binary BVH        (&.1 ms)\n", timer.Reset());
//     }

//     LOG_VERBOSE_SAMELINE("[CWBVH]Optimizing nodes");
//     ReinsertionOptimization();
//     LOG_VERBOSE_SAMELINE("[CWBVH]Optimizing nodes           (&.1 ms)\n", timer.Reset());

//     LOG_VERBOSE_SAMELINE("[CWBVH]Computing optimal SAH cost");
//     nodes2[node2Root].ComputeCost(nodes2.data());
//     LOG_VERBOSE_SAMELINE("[CWBVH]Computing optimal SAH cost (&.1 ms)\n", timer.Reset());

//     LOG_VERBOSE_SAMELINE("[CWBVH]Collapsing binary BVH");
//     wideTreeRoot = new Node8;
//     wideTreeRoot->parentAABB = nodes2[node2Root].aabb;
//     nodes2[node2Root].CollapseToNode8(nodes2.data(), wideTreeRoot, 8, 0);
//     LOG_VERBOSE_SAMELINE("[CWBVH]Collapsing binary BVH      (&.1 ms)\n", timer.Reset());

//     LOG_VERBOSE_SAMELINE("[CWBVH]Compressing wide BVH");
//     nodes.push_back(Node8Compressed());
//     wideTreeRoot->Compress(nodes, triangles, 0);
//     wideTreeRoot->Destory();
//     delete wideTreeRoot;
//     LOG_VERBOSE_SAMELINE("[CWBVH]Compressing wide BVH       (&.1 ms)\n", timer.Reset());

//     LOG("[CWBVH]Resulting Compressed Wide BVH has & nodes(&.2 MB), & primitives(&.2 MB)",
//                 nodes.size(), nodes.size() * sizeof(nodes[0]) / 1000000.0, triangles.size(),
//                 triangles.size() * sizeof(triangles[0]) / 1000000.0);
// }

// static float Distance(const CWBVH::Cluster& c0, const CWBVH::Cluster& c1) {
//     return Union(c0.aabb, c1.aabb).SurfaceArea();
// }
// static CWBVH::Cluster Merge(const CWBVH::Cluster& c0, const CWBVH::Cluster& c1) {
//     CWBVH::Cluster c;
//     c.aabb.Extend(c0.aabb);
//     c.aabb.Extend(c1.aabb);
//     c.children = new CWBVH::Cluster[2];
//     c.children[0] = c0;
//     c.children[1] = c1;
//     return c;
// }
// void CWBVH::BuildLocalOrderedBinaryBVH(const TriangleMesh* mesh) {
//     Timer timer;
//     AABB centroidAABB;
//     for (int i = 0; i < mesh->GetNumTriangles(); i++) {
//         Cluster c;
//         Triangle tri = mesh->GetTriangle(i);
//         c.triangle.v0 = tri.v0;
//         c.triangle.v1 = tri.v1;
//         c.triangle.v2 = tri.v2;
//         c.triangle.orginalIndex = i;
//         c.aabb = tri.GetAABB();
//         centroidAABB.Extend(c.aabb.Centroid());
//         clusters.push_back(c);
//     }

//     for (auto& cluster : clusters)
//         cluster.mortonCode = EncodeMorton32x3(centroidAABB.Offset(cluster.aabb.Centroid()));

//     pstd::sort(clusters.begin(), clusters.end(), [](const Cluster& lhs, const Cluster& rhs) {
//         return lhs.mortonCode < rhs.mortonCode;
//     });

//     int c = (int)clusters.size();
//     pstd::vector<int> N(clusters.size());
//     pstd::vector<int> P(clusters.size());

//     while (c > 1) {
//         ParallelFor(c, [&](int i) {
//             // Nearest neighbor search
//             float minDist = FloatMax;
//             for (int j = pstd::max(i - r, 0); j < pstd::min(i + r + 1, c); j++) {
//                 if (i != j) {
//                     float dist = Distance(clusters[i], clusters[j]);
//                     if (dist < minDist) {
//                         minDist = dist;
//                         N[i] = j;
//                     }
//                 }
//             }
//         });
//         for (int i = 0; i < c; i++) {
//             // Merging
//             if (N[N[i]] == i && i < N[i]) {
//                 clusters[i] = Merge(clusters[i], clusters[N[i]]);
//                 clusters[N[i]].invalid = true;
//             }
//         }
//         for (int i = 0; i < c; i++) {
//             // Compaction
//             P[i] = 0;
//             if (i != 0) {
//                 P[i] = P[i - 1];
//                 if (!clusters[i - 1].invalid)
//                     P[i]++;
//             }
//             if (!clusters[i].invalid)
//                 clusters[P[i]] = clusters[i];
//         }
//         c = P[c - 1] + !clusters[c - 1].invalid;
//     }
// }
// int CWBVH::Cluster::Flatten(pstd::vector<Node2>& nodes) const {
//     Node2 node;
//     node.index = nodes.size();
//     nodes.resize(nodes.size() + 1);

//     node.aabb = aabb;
//     if (children) {
//         node.children[0] = children[0].Flatten(nodes);
//         node.children[1] = children[1].Flatten(nodes);
//         nodes[node.children[0]].parent = node.index;
//         nodes[node.children[1]].parent = node.index;
//         nodes[node.children[0]].indexAsChild = 0;
//         nodes[node.children[1]].indexAsChild = 1;
//         node.isInternalNode = true;
//     } else {
//         node.triangle = triangle;
//         node.isInternalNode = false;
//     }

//     nodes[node.index] = node;
//     return node.index;
// }
// void CWBVH::Cluster::Destory() {
//     if (children) {
//         children[0].Destory();
//         children[1].Destory();
//         delete[] children;
//     }
// }

// void CWBVH::BuildBinnedBVH(const TriangleMesh* mesh) {
//     struct Primitive {
//         AABB aabb;
//         CompactTriangle triangle;
//     };
//     pstd::vector<Primitive> primitives(mesh->GetNumTriangles());
//     for (int i = 0; i < mesh->GetNumTriangles(); i++) {
//         Primitive prim;
//         Triangle tri = mesh->GetTriangle(i);
//         prim.triangle.v0 = tri.v0;
//         prim.triangle.v1 = tri.v1;
//         prim.triangle.v2 = tri.v2;
//         prim.triangle.orginalIndex = i;
//         prim.aabb = tri.GetAABB();
//         primitives[i] = prim;
//     }

//     auto BuildRecursively = [&](auto me, Primitive* begin, Primitive* end) {
//         Node2 node;
//         node.index = (int)nodes2.size();
//         nodes2.resize(nodes2.size() + 1);
//         int numPrimitives = int(end - begin);
//         CHECK_NE(numPrimitives, 0);

//         AABB aabbCentroid;
//         for (int i = 0; i < numPrimitives; i++) {
//             node.aabb.Extend(begin[i].aabb);
//             aabbCentroid.Extend(begin[i].aabb.Centroid());
//         }
//         if (numPrimitives == 1) {
//             node.isInternalNode = false;
//             node.triangle = begin->triangle;
//             nodes2[node.index] = node;
//             return node.index;
//         }
//         float surfaceArea = node.aabb.SurfaceArea();

//         Primitive* pmid = nullptr;
//         if (numPrimitives == 2) {
//             pmid = begin + numPrimitives / 2;
//         } else {
//             struct Bucket {
//                 int count = 0;
//                 AABB aabb;
//             };
//             const int nBuckets = 16;

//             float minCost = FloatMax;
//             int bestAxis = -1;
//             int splitBucket = -1;

//             for (int axis = 0; axis < 3; axis++) {
//                 if (!aabbCentroid.IsValid(axis))
//                     continue;

//                 Bucket buckets[nBuckets];

//                 for (int i = 0; i < numPrimitives; i++) {
//                     int b = nBuckets * aabbCentroid.Offset(begin[i].aabb.Centroid(axis), axis);
//                     if (b == nBuckets)
//                         b = nBuckets - 1;
//                     buckets[b].count++;
//                     buckets[b].aabb.Extend(begin[i].aabb);
//                 }

//                 float cost[nBuckets - 1] = {};

//                 AABB bForward;
//                 int countForward = 0;
//                 for (int i = 0; i < nBuckets - 1; i++) {
//                     bForward.Extend(buckets[i].aabb);
//                     countForward += buckets[i].count;
//                     cost[i] += countForward * bForward.SurfaceArea();
//                 }

//                 AABB bBackward;
//                 int countBackward = 0;
//                 for (int i = nBuckets - 1; i >= 1; i--) {
//                     bBackward.Extend(buckets[i].aabb);
//                     countBackward += buckets[i].count;
//                     cost[i - 1] += countBackward * bBackward.SurfaceArea();
//                 }

//                 for (int i = 0; i < nBuckets - 1; i++) {
//                     cost[i] = 0.5f + cost[i] / surfaceArea;
//                 }

//                 float axisMinCost = cost[0];
//                 int axisSplitBucket = 0;
//                 for (int i = 1; i < nBuckets - 1; i++) {
//                     if (cost[i] < axisMinCost) {
//                         axisMinCost = cost[i];
//                         axisSplitBucket = i;
//                     }
//                 }

//                 if (axisMinCost < minCost) {
//                     minCost = axisMinCost;
//                     bestAxis = axis;
//                     splitBucket = axisSplitBucket;
//                 }
//             }

//             pmid = pstd::partition(begin, end, [=](const Primitive& prim) {
//                 int b = nBuckets * aabbCentroid.Offset(prim.aabb.Centroid(bestAxis), bestAxis);
//                 if (b == nBuckets)
//                     b = nBuckets - 1;
//                 return b <= splitBucket;
//             });
//         }
//         if (pmid == begin || pmid == end) {
//             CHECK_LT(numPrimitives, 8);
//             pmid = begin + numPrimitives / 2;
//         }

//         node.isInternalNode = true;
//         node.children[0] = me(me, begin, pmid);
//         node.children[1] = me(me, pmid, end);
//         nodes2[node.children[0]].parent = node.index;
//         nodes2[node.children[1]].parent = node.index;
//         nodes2[node.children[0]].indexAsChild = 0;
//         nodes2[node.children[1]].indexAsChild = 1;

//         nodes2[node.index] = node;
//         return node.index;
//     };

//     BuildRecursively(BuildRecursively, primitives.data(), primitives.data() + primitives.size());
// }

// void CWBVH::BuildSweepSahBVH(const TriangleMesh* mesh) {
//     struct Primitive {
//         AABB aabb;
//         CompactTriangle triangle;
//     };
//     pstd::vector<Primitive> primitives(mesh->GetNumTriangles());
//     for (int i = 0; i < mesh->GetNumTriangles(); i++) {
//         Primitive prim;
//         Triangle tri = mesh->GetTriangle(i);
//         prim.aabb = tri.GetAABB();
//         prim.triangle.v0 = tri.v0;
//         prim.triangle.v1 = tri.v1;
//         prim.triangle.v2 = tri.v2;
//         prim.triangle.orginalIndex = i;
//         primitives[i] = prim;
//     }

//     auto BuildRecursively = [&](auto me, Primitive* begin, Primitive* end) {
//         Node2 node;
//         node.index = (int)nodes2.size();
//         nodes2.resize(nodes2.size() + 1);
//         for (Primitive* prim = begin; prim != end; prim++)
//             node.aabb.Extend(prim->aabb);
//         float surfaceArea = node.aabb.SurfaceArea();
//         int numPrimitives = int(end - begin);
//         int numSplits = numPrimitives - 1;
//         CHECK_GT(numPrimitives, 0);

//         if (numPrimitives == 1) {
//             node.isInternalNode = false;
//             node.triangle = begin->triangle;
//             nodes2[node.index] = node;
//             return node.index;
//         }

//         node.isInternalNode = true;

//         enum UseLowerOrUpperBound { Lower, Upper } useLowerOrUpperBound = Lower;
//         float bestCost = FloatMax;
//         int bestAxis = -1;
//         int bestSplitIndex = -1;

//         pstd::vector<float> costs(numSplits);
//         for (int axis = 0; axis < 3; axis++) {
//             {
//                 pstd::sort(begin, end, [&](const Primitive& l, const Primitive& r) {
//                     return l.aabb.lower[axis] < r.aabb.lower[axis];
//                 });
//                 AABB boundLeft;
//                 int countLeft = 0;
//                 for (int i = 0; i < numSplits; i++) {
//                     boundLeft.Extend(begin[i].aabb);
//                     countLeft++;
//                     costs[i] = boundLeft.SurfaceArea() * countLeft;
//                 }

//                 AABB boundRight;
//                 int countRight = 0;
//                 for (int i = numSplits - 1; i >= 0; i--) {
//                     boundRight.Extend(begin[i + 1].aabb);
//                     countRight++;
//                     costs[i] += boundRight.SurfaceArea() * countRight;
//                 }

//                 for (int i = 0; i < numSplits; i++) {
//                     float cost = 1.0f + costs[i] / surfaceArea;
//                     if (cost < bestCost) {
//                         bestCost = cost;
//                         bestAxis = axis;
//                         bestSplitIndex = i;
//                         useLowerOrUpperBound = Lower;
//                     }
//                 }
//             }

//             {
//                 pstd::sort(begin, end, [&](const Primitive& l, const Primitive& r) {
//                     return l.aabb.upper[axis] < r.aabb.upper[axis];
//                 });
//                 AABB boundLeft;
//                 int countLeft = 0;
//                 for (int i = 0; i < numSplits; i++) {
//                     boundLeft.Extend(begin[i].aabb);
//                     countLeft++;
//                     costs[i] = boundLeft.SurfaceArea() * countLeft;
//                 }
//                 AABB boundRight;
//                 int countRight = 0;
//                 for (int i = numSplits - 1; i >= 0; i--) {
//                     boundRight.Extend(begin[i + 1].aabb);
//                     countRight++;
//                     costs[i] += boundRight.SurfaceArea() * countRight;
//                 }

//                 for (int i = 0; i < numSplits; i++) {
//                     float cost = 1.0f + costs[i] / surfaceArea;
//                     if (cost < bestCost) {
//                         bestCost = cost;
//                         bestAxis = axis;
//                         bestSplitIndex = i;
//                         useLowerOrUpperBound = Upper;
//                     }
//                 }
//             }
//         }

//         Primitive* nth = begin + bestSplitIndex + 1;
//         if (useLowerOrUpperBound == Lower)
//             pstd::nth_element(begin, nth, end, [&](const Primitive& l, const Primitive& r) {
//                 return l.aabb.lower[bestAxis] < r.aabb.lower[bestAxis];
//             });
//         else if (useLowerOrUpperBound == Upper)
//             pstd::nth_element(begin, nth, end, [&](const Primitive& l, const Primitive& r) {
//                 return l.aabb.upper[bestAxis] < r.aabb.upper[bestAxis];
//             });
//         else
//             CHECK(false);

//         node.children[0] = me(me, begin, nth);
//         node.children[1] = me(me, nth, end);
//         nodes2[node.children[0]].parent = node.index;
//         nodes2[node.children[1]].parent = node.index;
//         nodes2[node.children[0]].indexAsChild = 0;
//         nodes2[node.children[1]].indexAsChild = 1;
//         nodes2[node.index] = node;
//         return node.index;
//     };

//     BuildRecursively(BuildRecursively, primitives.data(), primitives.data() + primitives.size());
// }

// struct Item {
//     Item(int nodeIndex, float costInduced, float inverseCost)
//         : nodeIndex(nodeIndex), costInduced(costInduced), inverseCost(inverseCost){};

//     friend bool operator<(Item l, Item r) {
//         return l.inverseCost < r.inverseCost;
//     }

//     int nodeIndex;
//     float costInduced;
//     float inverseCost;
// };
// struct Pair {
//     friend bool operator<(Pair l, Pair r) {
//         return l.inefficiency > r.inefficiency;
//     }

//     int index = -1;
//     float inefficiency;
// };
// void CWBVH::ReinsertionOptimization() {
//     auto FindNodeForReinsertion = [&](int nodeIndex) {
//         float eps = 1e-20f;
//         float costBest = FloatMax;
//         int nodeBest = -1;

//         auto& L = nodes2[nodeIndex];

//         pstd::priority_queue<Item> queue;
//         queue.push(Item(node2Root, 0.0f, FloatMax));

//         while (queue.size()) {
//             Item item = queue.top();
//             queue.pop();
//             auto CiLX = item.costInduced;
//             auto& X = nodes2[item.nodeIndex];

//             if (CiLX + L.SurfaceArea() >= costBest)
//                 break;

//             float CdLX = Union(X.aabb, L.aabb).SurfaceArea();
//             float CLX = CiLX + CdLX;
//             if (CLX < costBest) {
//                 costBest = CLX;
//                 nodeBest = item.nodeIndex;
//             }

//             float Ci = CLX - X.SurfaceArea();

//             if (Ci + L.SurfaceArea() < costBest) {
//                 if (X.isInternalNode) {
//                     CHECK_NE(X.children[0], -1);
//                     CHECK_NE(X.children[1], -1);
//                     CHECK(!nodes2[X.children[0]].removed);
//                     CHECK(!nodes2[X.children[1]].removed);
//                     queue.push(Item(X.children[0], Ci, 1.0f / (Ci + eps)));
//                     queue.push(Item(X.children[1], Ci, 1.0f / (Ci + eps)));
//                 }
//             }
//         }

//         return nodeBest;
//     };

//     RNG sampler;
//     float startCost = nodes2[node2Root].ComputeSAHCost(nodes2.data()) / 100000.0f;
//     float lastCost = startCost;
//     int numConvergedPasses = 0;
//     for (int pass = 0; pass < 256; pass++) {
//         if (pass % 5 == 1) {
//             float cost = nodes2[node2Root].ComputeSAHCost(nodes2.data()) / 100000.0f;
//             LOG_SAMELINE("[CWBVH]SAH cost after & optimization passes: &", pass, cost);
//             if (cost < lastCost * 0.99f) {
//                 numConvergedPasses = 0;
//                 lastCost = cost;
//             } else {
//                 numConvergedPasses++;
//                 if (numConvergedPasses > 1) {
//                     LOG_SAMELINE(
//                         "[CWBVH]SAH cost converged to &.1(&.1%) after & optimization passes\n",
//                         cost, 100.0f * cost / startCost, pass);
//                     break;
//                 }
//             }
//         }

//         pstd::vector<pstd::pair<int, int>> candicates;
//         if (pass % 3 == 0) {
//             pstd::vector<Pair> inefficiencies(nodes2.size());

//             for (int i = 0; i < (int)nodes2.size(); i++)
//                 inefficiencies[i] = {i, nodes2[i].Inefficiency(nodes2.data())};

//             pstd::partial_sort(inefficiencies.begin(), inefficiencies.begin() + nodes2.size() /
//             200,
//                               inefficiencies.end());
//             for (int i = 0; i < (int)nodes2.size() / 200; i++) {
//                 int nodeIndex = inefficiencies[i].index;
//                 if (!nodes2[nodeIndex].isInternalNode || nodes2[nodeIndex].removed ||
//                     nodes2[nodeIndex].parent == -1 || nodes2[nodes2[nodeIndex].parent].removed ||
//                     nodes2[nodes2[nodeIndex].parent].parent == -1 ||
//                     nodes2[nodes2[nodes2[nodeIndex].parent].parent].removed ||
//                     nodes2[nodes2[nodes2[nodeIndex].parent].parent].parent == -1)
//                     continue;

//                 Node2& node = nodes2[nodeIndex];
//                 Node2& parent = nodes2[node.parent];
//                 Node2& grandparent = nodes2[parent.parent];
//                 node.removed = true;
//                 parent.removed = true;

//                 int secondChildOfParent = parent.children[1 - node.indexAsChild];
//                 grandparent.children[parent.indexAsChild] = secondChildOfParent;
//                 nodes2[secondChildOfParent].indexAsChild = parent.indexAsChild;
//                 nodes2[secondChildOfParent].parent = grandparent.index;
//                 grandparent.UpdateAABB(nodes2.data());

//                 nodes2[node.children[0]].removed = true;
//                 nodes2[node.children[1]].removed = true;

//                 candicates.push_back({node.children[0], nodeIndex});
//                 candicates.push_back({node.children[1], node.parent});
//             }
//         } else {
//             for (int i = 0; i < (int)nodes2.size() / 100; i++) {
//                 int nodeIndex = -1;
//                 do {
//                     nodeIndex = sampler.Uniform64u() % nodes2.size();
//                 } while (!nodes2[nodeIndex].isInternalNode || nodes2[nodeIndex].removed ||
//                          nodes2[nodeIndex].parent == -1 ||
//                          nodes2[nodes2[nodeIndex].parent].removed ||
//                          nodes2[nodes2[nodeIndex].parent].parent == -1 ||
//                          nodes2[nodes2[nodes2[nodeIndex].parent].parent].removed ||
//                          nodes2[nodes2[nodes2[nodeIndex].parent].parent].parent == -1);

//                 Node2& node = nodes2[nodeIndex];
//                 Node2& parent = nodes2[node.parent];
//                 Node2& grandparent = nodes2[parent.parent];
//                 node.removed = true;
//                 parent.removed = true;

//                 int secondChildOfParent = parent.children[1 - node.indexAsChild];
//                 grandparent.children[parent.indexAsChild] = secondChildOfParent;
//                 nodes2[secondChildOfParent].indexAsChild = parent.indexAsChild;
//                 nodes2[secondChildOfParent].parent = grandparent.index;
//                 grandparent.UpdateAABB(nodes2.data());

//                 nodes2[node.children[0]].removed = true;
//                 nodes2[node.children[1]].removed = true;

//                 candicates.push_back({node.children[0], nodeIndex});
//                 candicates.push_back({node.children[1], node.parent});
//             }
//         }

//         pstd::sort(candicates.begin(), candicates.end(),
//                   [&](pstd::pair<int, int> l, pstd::pair<int, int> r) {
//                       return nodes2[l.first].SurfaceArea() > nodes2[r.first].SurfaceArea();
//                   });

//         for (pstd::pair<int, int> node : candicates) {
//             int C = node.first;
//             int N = node.second;
//             Node2& x = nodes2[FindNodeForReinsertion(C)];
//             Node2& n = nodes2[N];
//             Node2& c = nodes2[C];

//             if (x.parent != -1)
//                 nodes2[x.parent].children[x.indexAsChild] = n.index;
//             else
//                 node2Root = n.index;
//             n.parent = x.parent;
//             n.indexAsChild = x.indexAsChild;
//             n.removed = false;

//             n.children[0] = x.index;
//             n.children[1] = c.index;
//             x.parent = n.index;
//             c.parent = n.index;
//             x.indexAsChild = 0;
//             c.indexAsChild = 1;
//             c.removed = false;
//             n.UpdateAABB(nodes2.data());
//         }
//     }
// }

// void CWBVH::Node2::ComputeCost(Node2* nodes, int depth) {
//     if (isInternalNode) {
//         if (depth < 6) {
//             ParallelFor(2, [&](int i) { nodes[children[i]].ComputeCost(nodes, depth + 1); });
//         } else {
//             nodes[children[0]].ComputeCost(nodes, depth + 1);
//             nodes[children[1]].ComputeCost(nodes, depth + 1);
//         }

//         float minCost = FloatMax;
//         for (int i = 1; i <= 7; i++) {
//             cost[i] = CostNode(nodes, i);
//             if (cost[i] < minCost) {
//                 minCost = cost[i];
//             }
//         }
//     } else {
//         for (int i = 1; i <= 7; i++)
//             cost[i] = aabb.SurfaceArea() * kCostPrimitive;
//     }
// }
// float CWBVH::Node2::CostNode(Node2* nodes, int i) {
//     if (i == 1) {
//         float costLeaf = CostLeaf(nodes);
//         float costInternal = CostInternal(nodes);
//         if (costLeaf < costInternal) {
//             createLeafOrInternal = NodeType::Leaf;
//             return costLeaf;
//         } else {
//             createLeafOrInternal = NodeType::Internal;
//             return costInternal;
//         }
//     } else {
//         float costDistribute = CostDistribute(nodes, i);
//         float costCreateFewerNode = CostNode(nodes, i - 1);
//         if (costDistribute < costCreateFewerNode) {
//             return costDistribute;
//         } else {
//             return costCreateFewerNode;
//         }
//     }
// }
// float CWBVH::Node2::CostLeaf(Node2* nodes) const {
//     int Pn = NumPrimitives(nodes);
//     if (Pn <= kNodeMaxNumPrimitives)
//         return aabb.SurfaceArea() * Pn * kCostPrimitive;
//     else
//         return FloatMax;
// }
// float CWBVH::Node2::CostInternal(Node2* nodes) {
//     return CostDistribute(nodes, 8) + aabb.SurfaceArea() * kCostNode;
// }
// float CWBVH::Node2::CostDistribute(Node2* nodes, int j) {
//     float minCost = FloatMax;
//     for (int k = 1; k < j; k++) {
//         float cost = nodes[children[0]].cost[k] + nodes[children[1]].cost[j - k];
//         if (cost < minCost) {
//             minCost = cost;
//             distributeLeft[j] = k;
//             distributeRight[j] = j - k;
//         }
//     }
//     return minCost;
// }
// int CWBVH::Node2::NumPrimitives(Node2* nodes) const {
//     if (isInternalNode) {
//         int n = 0;
//         n += nodes[children[0]].NumPrimitives(nodes);
//         n += nodes[children[1]].NumPrimitives(nodes);
//         return n;
//     } else {
//         return 1;
//     }
// }
// void CWBVH::Node2::CollapseToNode8(Node2* nodes2, Node8* node8, int numRoots, int index) {
//     CHECK_LT(index, 8);
//     if (numRoots == 1 || !isInternalNode) {
//         node8->aabb[index] = aabb;
//         if (createLeafOrInternal == NodeType::Leaf || !isInternalNode) {
//             if (!isInternalNode) {
//                 node8->triangles[index].push_back(triangle);
//             } else if (isInternalNode) {
//                 if (nodes2[children[0]].isInternalNode) {
//                     node8->triangles[index].push_back(
//                         nodes2[nodes2[children[0]].children[0]].triangle);
//                     node8->triangles[index].push_back(
//                         nodes2[nodes2[children[0]].children[1]].triangle);
//                 } else {
//                     node8->triangles[index].push_back(nodes2[children[0]].triangle);
//                 }

//                 if (nodes2[children[1]].isInternalNode) {
//                     node8->triangles[index].push_back(
//                         nodes2[nodes2[children[1]].children[0]].triangle);
//                     node8->triangles[index].push_back(
//                         nodes2[nodes2[children[1]].children[1]].triangle);
//                 } else {
//                     node8->triangles[index].push_back(nodes2[children[1]].triangle);
//                 }
//             }
//         } else {
//             CHECK_NE(distributeLeft[8], 0);
//             CHECK_NE(distributeRight[8], 0);
//             Node8* child = new Node8;
//             child->parentAABB = aabb;
//             nodes2[children[0]].CollapseToNode8(nodes2, child, distributeLeft[8], 0);
//             nodes2[children[1]].CollapseToNode8(nodes2, child, distributeRight[8],
//                                                 distributeLeft[8]);
//             node8->children[index] = child;
//         }
//     } else {
//         CHECK_NE(distributeLeft[numRoots], 0);
//         CHECK_NE(distributeRight[numRoots], 0);
//         nodes2[children[0]].CollapseToNode8(nodes2, node8, distributeLeft[numRoots], index);
//         nodes2[children[1]].CollapseToNode8(nodes2, node8, distributeRight[numRoots],
//                                             index + distributeLeft[numRoots]);
//     }
// }

// void CWBVH::Node8::Compress(pstd::vector<Node8Compressed>& nodes,
//                             pstd::vector<CompactTriangle>& triangles, uint32_t index) {
//     CHECK(parentAABB.IsValid());
//     nodes[index].SetParentAABB(parentAABB);

//     nodes[index].childBaseIndex = nodes.size();
//     nodes[index].triangleBaseIndex = triangles.size();

//     // Reordering children
//     float cost[8][8];
//     pstd::vector<int> rows = {0, 1, 2, 3, 4, 5, 6, 7};
//     pstd::vector<int> cols = {0, 1, 2, 3, 4, 5, 6, 7};
//     pstd::vector<vec2i> slots;

//     for (int i : rows)
//         for (int s : cols) {
//             vec3 d;
//             for (int b = 0; b < 3; b++)
//                 d[b] = (1 & (s >> b)) ? -1 : 1;
//             cost[i][s] =
//                 aabb[i].IsValid() ? Dot(aabb[i].Centroid() - parentAABB.Centroid(), d) : 0.0f;
//         }

//     while (slots.size() != 8) {
//         int minRow = -1, minCol = -1;
//         float minCost = FloatMax;
//         for (int i : rows)
//             for (int s : cols) {
//                 if (cost[i][s] < minCost) {
//                     minCost = cost[i][s];
//                     minRow = i;
//                     minCol = s;
//                 }
//             }
//         slots.push_back({minRow, minCol});
//         rows.erase(pstd::find(rows.begin(), rows.end(), minRow));
//         cols.erase(pstd::find(cols.begin(), cols.end(), minCol));
//     }

//     AABB oldAABB[8];
//     Node8* oldChildren[8];
//     pstd::vector<CompactTriangle> oldTriangles[8];
//     for (int i = 0; i < 8; i++) {
//         oldAABB[i] = aabb[i];
//         oldChildren[i] = this->children[i];
//         oldTriangles[i] = this->triangles[i];
//         this->children[i] = nullptr;
//         this->triangles[i] = {};
//     }
//     for (int i = 0; i < (int)slots.size(); i++) {
//         aabb[slots[i].y] = oldAABB[slots[i].x];
//         this->children[slots[i].y] = oldChildren[slots[i].x];
//         this->triangles[slots[i].y] = oldTriangles[slots[i].x];
//     }

//     for (int i = 0; i < 8; i++)
//         if (children[i])
//             nodes.push_back(Node8Compressed());
//     for (int i = 0; i < 8; i++)
//         for (const auto& t : this->triangles[i])
//             triangles.push_back(t);

//     int triangleNumOffset = 0;
//     for (int i = 0; i < 8; i++) {
//         nodes[index].SetChildAABB(i, aabb[i]);
//         if (children[i]) {
//             nodes[index].SetChildAsNode(i);
//         } else if (this->triangles[i].size()) {
//             nodes[index].SetChildAsTriangle(i, triangleNumOffset, this->triangles[i].size());
//             triangleNumOffset += this->triangles[i].size();
//         }
//     }

//     int childOffset = 0;
//     for (int i = 0; i < 8; i++) {
//         if (children[i])
//             children[i]->Compress(nodes, triangles, nodes[index].childBaseIndex + childOffset++);
//     }
// }
// void CWBVH::Node8::Destory() {
//     for (int i = 0; i < 8; i++) {
//         if (children[i]) {
//             children[i]->Destory();
//             delete children[i];
//         }
//     }
// }

// void CWBVH::Node8Compressed::SetParentAABB(AABB B) {
//     p = B.lower;
//     for (int i = 0; i < 3; i++) {
//         e[i] = pstd::ceil(pstd::log2(B.Diagonal()[i] / ((1 << 8) - 1)));
//     }
// }
// void CWBVH::Node8Compressed::SetChildAABB(int index, AABB b) {
//     static constexpr auto Exp2 = [](int8_t e) {
//         union {
//             float f;
//             uint32_t u = 0;
//         };
//         u |= (e + 127) << 23;
//         return f;
//     };
//     for (int i = 0; i < 3; i++) {
//         qlohi[i][0][index] = pstd::floor((b.lower[i] - p[i]) / Exp2(e[i]));
//         qlohi[i][1][index] = pstd::ceil((b.upper[i] - p[i]) / Exp2(e[i]));
//     }
// }

// static inline uint64_t sign_extend_s8x8(uint64_t x) {
//     x |= x >> 1;
//     x |= x >> 2;
//     x |= x >> 4;
//     return x;
// }
// static inline int highest_set_bit(uint32_t x) {
//     // return 31 - __builtin_clz(x);

//     union {
//         float f;
//         uint32_t i;
//     };
//     f = (float)x;

//     return (0xff & (i >> 23)) - 127;
// }
// static inline int popc(uint32_t x) {
//     // return __builtin_popcount(x);

//     x = x - ((x >> 1) & 0x55555555);
//     x = (x & 0x33333333) + ((x >> 2) & 0x33333333);
//     x = (x + (x >> 4)) & 0x0f0f0f0f;
//     return (x * 0x01010101) >> 24;
// }
// static inline uint64_t extract_byte(uint64_t x, int n) {
//     return (x >> (n << 3)) & 0xff;
// }
// static inline float exp2i8(int8_t e) {
//     union {
//         float f;
//         uint32_t i = 0;
//     };
//     i |= (e + 127) << 23;
//     return f;
// }

// bool CWBVH::Hit(Ray) const {
//     return false;
// }
// bool CWBVH::Intersect(Ray& ray, Interaction& it) const {
//     if (ray.tmax <= 0.0f)
//         return false;

//     bool hit = false;

//     vec3 invdir = SafeRcp(ray.d);
//     int octant = 0;
//     for (int b = 0; b < 3; b++)
//         octant |= (invdir[b] < 0) << b;
//     uint64_t octinv8 = (7 - octant) * 0x0101010101010101;
//     const bool octl[3] = {pstd::signbit(ray.d[0]), pstd::signbit(ray.d[1]),
//     pstd::signbit(ray.d[2])}; const bool octh[3] = {!octl[0], !octl[1], !octl[2]};

//     const Node8Compressed* PINE_RESTRICT nodes = this->nodes.data();
//     const CompactTriangle* PINE_RESTRICT triangles = this->triangles.data();

//     int closestTriangleIndex = -1;
//     int triangleIndex = -1;

//     auto IntersectBbox = [octl = octl, octh = octh](const Node8Compressed& node, int nodeIndex,
//                                                     vec3 org, vec3 dir, float tmin, float tmax) {
//         float tmin0 = node.qlohi[0][octl[0]][nodeIndex] * dir[0] + org[0];
//         float tmax0 = node.qlohi[0][octh[0]][nodeIndex] * dir[0] + org[0];
//         float tmin1 = node.qlohi[1][octl[1]][nodeIndex] * dir[1] + org[1];
//         float tmax1 = node.qlohi[1][octh[1]][nodeIndex] * dir[1] + org[1];
//         float tmin2 = node.qlohi[2][octl[2]][nodeIndex] * dir[2] + org[2];
//         float tmax2 = node.qlohi[2][octh[2]][nodeIndex] * dir[2] + org[2];
//         return max(max(max(tmin, tmin0), tmin1), tmin2) <= min(min(min(tmax, tmax0), tmax1),
//         tmax2);
//     };
//     auto GetClosestNode = [&](uint64_t& G) {
//         int bit_index = highest_set_bit(G & 0xff000000);
//         G ^= 1u << bit_index;
//         uint32_t slot_index = (bit_index - 24) ^ (7 - octant);
//         uint32_t imask = G & 0x000000ff;
//         uint32_t relative_index = popc(imask & ~(uint32_t(-1) << slot_index));
//         return nodes[(G >> 32) + relative_index];
//     };
//     auto GetNextTriangle = [&](uint64_t& Gt) {
//         int bit_index = highest_set_bit(Gt & 0x00ffffff);
//         Gt ^= 1u << bit_index;
//         triangleIndex = (Gt >> 32) + bit_index;
//         return triangles[triangleIndex];
//     };
//     auto IntersectTriangles = [&](uint64_t& Gt) {
//         while (Gt & 0x00ffffff) {
//             auto t = GetNextTriangle(Gt);
//             if (Triangle::Intersect(ray, it, t.v0, t.v1, t.v2)) {
//                 closestTriangleIndex = triangleIndex;
//                 hit = true;
//             }
//         }
//     };

//     uint64_t stack[32];
//     int ptr = 0;
//     uint64_t G = 0;
//     uint64_t Gt = 0;

//     bool isFirstIteration = true;
//     while (true) {
//         if (PINE_UNLIKELY(Gt))
//             IntersectTriangles(Gt);

//         if (PINE_LIKELY(G & 0xff000000 || isFirstIteration)) {
//             // Node group entry
//             it.bvh += 8;
//             Node8Compressed node = isFirstIteration ? nodes[0] : GetClosestNode(G);

//             if (G & 0xff000000)
//                 stack[ptr++] = G;

//             vec3 org = (node.p - ray.o) * invdir;
//             vec3 dir = {exp2i8(node.e[0]) * invdir[0], exp2i8(node.e[1]) * invdir[1],
//                         exp2i8(node.e[2]) * invdir[2]};

//             uint32_t hitmask = 0;

//             uint64_t meta8 = node.meta8;
//             uint64_t is_inner8 = (meta8 & (meta8 << 1)) & 0x1010101010101010;
//             uint64_t inner_mask8 = sign_extend_s8x8(is_inner8 << 3);
//             uint64_t bit_index8 = (meta8 ^ (octinv8 & inner_mask8)) & 0x1f1f1f1f1f1f1f1f;
//             uint64_t child_bits8 = (meta8 >> 5) & 0x0707070707070707;
// #pragma unroll
//             for (int j = 0; j < 8; j++) {
//                 if (IntersectBbox(node, j, org, dir, ray.tmin, ray.tmax)) {
//                     uint32_t child_bits = extract_byte(child_bits8, j);
//                     int bit_index = extract_byte(bit_index8, j);
//                     hitmask = hitmask | (child_bits << bit_index);
//                 }
//             }

//             G = (uint64_t(node.childBaseIndex) << 32) | (hitmask & 0xff000000) | node.imask;
//             Gt = (uint64_t(node.triangleBaseIndex) << 32) | hitmask;
//         } else {
//             // Triangle group entry
//             Gt = G;
//             G = 0;
//         }

//         IntersectTriangles(Gt);

//         if ((G & 0xff000000) == 0) {
//             if (PINE_UNLIKELY(ptr == 0))
//                 break;
//             G = stack[--ptr];
//         }

//         isFirstIteration = false;
//     }

//     if (hit) {
//         const auto& tri = mesh->GetTriangle(triangles[closestTriangleIndex].orginalIndex);
//         it.p = tri.InterpolatePosition(it.uv);
//         it.n = Normalize(Cross(tri.v0 - tri.v1, tri.v0 - tri.v2));
//         tri.ComputeDpDuv(it.dpdu, it.dpdv);
//     }

//     return hit;
// }

// }  // namespace pine