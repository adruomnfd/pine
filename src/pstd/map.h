#ifndef PINE_STD_MAP_H
#define PINE_STD_MAP_H

#include <pstd/vector.h>
#include <pstd/tuple.h>

namespace pstd {

template <typename Key, typename Value, typename Pred = less<Key>>
class map {
  public:
    template <typename Pair = pair<Key, Value>>
    auto insert(Pair&& p) {
        return xs.push_back(p);
    }

    auto find(const Key& key) const {
        for (auto it = xs.begin(); it != xs.end(); ++it)
            if (it->first == key)
                return it;

        return xs.end();
    }

    Value& operator[](const Key& key) {
        auto it = find(key);
        if (it == xs.end())
            xs.push_back(pair<Key, Value>{key, {}});
        return xs.back().second;
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

}  // namespace pstd

#endif  // PINE_STD_MAP_H