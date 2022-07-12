#ifndef PINE_STD_MAP_H
#define PINE_STD_MAP_H

#include <pstd/vector.h>
#include <pstd/tuple.h>

namespace pstd {

template <typename Key, typename Value, typename Pred = less<Key>>
class map {
  public:
    struct Node {
        pair<Key, Value> key_value;
        Node* children[2] = {};
        Node* parent = nullptr;
    };

    struct iterator {
        pair<Key, Value>& operator*() const {
            return node->key_value;
        }
        pair<Key, Value>* operator->() const {
            return &node->key_value;
        }

        iterator& operator++() {
            if (node->children[0]) {
                node = node->children[0];
            } else if (node->children[1]) {
                node = node->children[1];
            } else {
                while (true) {
                    if (!node->parent) {
                        node = nullptr;
                        break;
                    } else if (auto right = node->parent->children[1]; right && right != node) {
                        node = right;
                        break;
                    }
                    node = node->parent;
                }
            }

            return *this;
        }

        iterator& operator++(int) {
            auto old = *this;
            ++(*this);
            return old;
        }

        friend bool operator==(iterator lhs, iterator rhs) {
            return lhs.node == rhs.node;
        }
        friend bool operator!=(iterator lhs, iterator rhs) {
            return lhs.node != rhs.node;
        }

        Node* node = nullptr;
    };

    ~map() {
        clear();
    }

    map() = default;

    map(const map& rhs) : map() {
        *this = rhs;
    }
    map(map&& rhs) : map() {
        *this = pstd::move(rhs);
    }
    map& operator=(const map& rhs) {
        clear();
        size_ = rhs.size();
        root = copy(rhs.root);
        return *this;
    }
    map& operator=(map&& rhs) {
        pstd::swap(root, rhs.root);
        pstd::swap(size_, rhs.size_);
        return *this;
    }

    template <typename Pair = pair<Key, Value>>
    void insert(Pair&& p) {
        Node* parent = nullptr;
        Node** node = &root;
        while (*node) {
            parent = *node;
            node = &next(*node, p.first);
        }

        *node = new Node{pstd::forward<Pair>(p), {}, parent};
        ++size_;
    }

    auto find(const Key& key) const {
        Node* node = root;
        while (node) {
            if (node->key_value.first == key)
                return iterator{node};
            node = next(node, key);
        }

        return end();
    }

    Value& operator[](const Key& key) {
        Node* parent = nullptr;
        Node** node = &root;
        while (*node) {
            Node* n = *node;
            if (n->key_value.first == key)
                return n->key_value.second;
            parent = *node;
            node = &next(n, key);
        }

        *node = new Node{{key, {}}, {}, parent};
        ++size_;
        return (*node)->key_value.second;
    }

    auto begin() {
        return iterator{root};
    }
    auto end() {
        return iterator{};
    }

    auto begin() const {
        return iterator{root};
    }
    auto end() const {
        return iterator{};
    }

    size_t size() const {
        return size_;
    }

    void clear() {
        if (!root)
            return;
        Node* stack[64];
        int p = 0;

        stack[p++] = root;
        while (p) {
            Node* node = stack[--p];
            if (node->children[0])
                stack[p++] = node->children[0];
            if (node->children[1])
                stack[p++] = node->children[1];
            delete node;
        }

        size_ = 0;
    }

  private:
    Node*& next(Node* node, const Key& key) const {
        return pred(key, node->key_value.first) ? node->children[0] : node->children[1];
    }

    static Node* copy(Node* node) {
        if (node) {
            Node* c0 = copy(node->children[0]);
            Node* c1 = copy(node->children[1]);
            Node* n = new Node{node->key_value, {c0, c1}, {}};
            if (c0)
                c0->parent = n;
            if (c1)
                c1->parent = n;
            return n;
        } else {
            return nullptr;
        }
    }

    Node* root = nullptr;
    size_t size_ = 0;
    Pred pred;
};

}  // namespace pstd

#endif  // PINE_STD_MAP_H