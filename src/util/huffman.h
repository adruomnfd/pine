#ifndef PINE_UTIL_HUFFMAN_H

#include <core/math.h>

#include <unordered_map>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <vector>

namespace pine {

template <typename T>
T Id(const T& value) {
    return value;
}

template <typename T, typename F>
void Appendf(std::vector<T>& a, const std::vector<T>& b, F f) {
    for (auto& x : b)
        a.push_back(f(x));
}

template <typename T>
void Append(std::vector<T>& a, const std::vector<T>& b) {
    return Appendf(a, b, Id<T>);
}

template <typename T>
struct HuffmanTree {
    mutable std::unordered_map<T, uint32_t> encoder;
};

template <typename Input>
auto ComputeOccurrences(const Input& input) {
    using T = typename Input::value_type;
    std::unordered_map<T, int> result;
    for (auto& value : input)
        result[value]++;
    return std::vector<std::pair<T, int>>(begin(result), end(result));
}

template <typename Input>
auto BuildHuffmanTree(const Input& input) {
    using T = typename Input::value_type;
    if (input.size() == 0)
        return HuffmanTree<T>{};

    auto occurrences = ComputeOccurrences(input);

    struct Node {
        int frequency = 0;
        std::vector<uint32_t> paths;
        std::vector<uint32_t> indices;
    };

    std::vector<Node> nodes(occurrences.size());
    for (size_t i = 0; i < occurrences.size(); i++) {
        nodes[i].frequency = occurrences[i].second;
        nodes[i].indices = {(uint32_t)i};
        nodes[i].paths = {1};
    }

    auto predicate = [](auto& l, auto& r) { return l.frequency > r.frequency; };

    sort(begin(nodes), end(nodes), predicate);

    while (nodes.size() != 1) {
        const Node &n0 = nodes[nodes.size() - 1], &n1 = nodes[nodes.size() - 2];
        Node node;
        node.paths.reserve(n0.paths.size() + n1.paths.size());
        node.indices.reserve(n0.indices.size() + n1.indices.size());

        node.frequency = n0.frequency + n1.frequency;
        Appendf(node.paths, n0.paths, [](uint32_t p) { return (p << 1) + 0; });
        Appendf(node.paths, n1.paths, [](uint32_t p) { return (p << 1) + 1; });
        Append(node.indices, n0.indices);
        Append(node.indices, n1.indices);
        nodes.resize(nodes.size() - 2);
        nodes.insert(lower_bound(begin(nodes), end(nodes), node, predicate), node);
    }

    HuffmanTree<T> tree;
    for (size_t i = 0; i < nodes[0].paths.size(); i++)
        tree.encoder[occurrences[nodes[0].indices[i]].first] = nodes[0].paths[i];

    return tree;
}

struct HuffmanEncoded {
    std::vector<uint8_t> data;
    uint32_t nbits = 0;
    size_t nElements = 0;
};

template <typename T, typename Input>
HuffmanEncoded HuffmanEncode(const HuffmanTree<T>& tree, const Input& input) {
    std::vector<uint8_t> output;
    uint8_t bits = 0;
    int occupanied = 0;

    for (auto& x : input) {
        uint32_t ex = tree.encoder[x];
        int h = HighestSetBit(ex);
        ex ^= 1u << h;

        while (h) {
            bits |= ex << occupanied;
            int used = std::min(h, 8 - occupanied);
            ex >>= used;
            h -= used;
            occupanied += used;

            if (occupanied == 8) {
                output.push_back(bits);
                bits = 0;
                occupanied = 0;
            }
        }
    }

    uint32_t nbits = output.size() * 8 + occupanied;

    if (occupanied != 0)
        output.push_back(bits);

    return {output, nbits, input.size()};
}

template <typename Output, typename T>
Output HuffmanDecode(const HuffmanTree<T>& tree, const HuffmanEncoded& encoded) {
    Output output;
    if (tree.encoder.size() <= 1) {
        output.resize(encoded.nElements);
        for (size_t i = 0; i < encoded.nElements; i++)
            output[i] = tree.encoder.begin()->first;
        return output;
    }

    struct Path {
        uint32_t path;
        uint32_t index;
    };
    std::vector<T> values;
    std::vector<Path> paths;
    values.reserve(tree.encoder.size());
    paths.reserve(tree.encoder.size());
    for (auto& pair : tree.encoder) {
        values.push_back(pair.first);
        paths.push_back({pair.second, (uint32_t)paths.size()});
    }

    std::vector<uint32_t> indices(paths.size() * 2);

    auto build = [&](auto me, Path* first, Path* last, uint32_t index) -> void {
        if (first + 1 == last) {
            if (index >= (uint32_t)indices.size())
                indices.resize(RoundUpPow2(index));
            indices[index] = first->index + 1;
        } else {
            Path* pmid = std::partition(first, last, [](Path n) { return (n.path & 1) == 0; });
            for (Path* it = first; it != last; ++it)
                it->path >>= 1;
            me(me, first, pmid, index * 2 + 1);
            me(me, pmid, last, index * 2 + 2);
        }
    };

    build(build, paths.data(), paths.data() + paths.size(), 0);

    uint8_t bits = 0;
    int used = 8;
    uint32_t bitsArrayIndex = 0, currentBitIndex = 0;

    auto next_bit = [&]() {
        if (used == 8) {
            used = 0;
            bits = encoded.data[bitsArrayIndex++];
        }
        ++currentBitIndex;

        return 1 & (bits >> used++);
    };
    auto nomore = [&]() { return currentBitIndex == encoded.nbits; };

    int nodeIndex = 0;

    output.reserve(encoded.nElements);
    while (!nomore()) {
        nodeIndex = nodeIndex * 2 + (next_bit() ? 2 : 1);
        uint32_t index = indices[nodeIndex];
        if (index) {
            output.push_back(values[index - 1]);
            nodeIndex = 0;
        }
    }

    return output;
}

}  // namespace pine

#endif  // PINE_UTIL_HUFFMAN_H
