#ifndef PINE_STD_MAP_H
#define PINE_STD_MAP_H

#include <pstd/vector.h>
#include <pstd/tuple.h>

namespace pstd {

template <typename Key, typename Value, typename Pred = less<Key>>
class map {
    template <typename Key2, typename Value2, typename Pred2>
    friend inline string to_string(const map<Key2, Value2, Pred2>& map);

  public:
    template <typename Pair>
    auto insert(Pair&& p) {
        auto it = lower_bound(xs.begin(), xs.end(), [&](auto& x) { return pred(x.first, p.first); });

        return xs.insert(it, p);
    }

    auto find(const Key& key) const {
        auto it = lower_bound(xs.begin(), xs.end(), [&](auto& x) { return pred(x.first, key); });

        if (it->first != key)
            it = xs.end();

        return it;
    }

    Value& operator[](const Key& key) {
        auto it = lower_bound(xs.begin(), xs.end(), [&](auto& x) { return pred(x.first, key); });

        if (it == xs.end() || it->first != key)
            it = insert(pair<Key, Value>{key, {}});

        return it->second;
    }

    auto begin() {
        return xs.begin();
    }
    auto end() {
        return xs.end();
    }

    auto begin() const {
        return xs.begin();
    }
    auto end() const {
        return xs.end();
    }

  private:
    vector<pair<Key, Value>> xs;
    Pred pred;
};

template <typename Key, typename Value, typename Pred>
inline string to_string(const map<Key, Value, Pred>& map) {
    return to_string(map.xs);
}

}  // namespace pstd

#endif  // PINE_STD_MAP_H