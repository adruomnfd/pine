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
        return base.push_back(pstd::forward<Pair>(p));
    }

    auto find(const Key& key) const {
        // TODO
        for (auto it = begin(); it != end(); ++it)
            if (it->first == key)
                return it;
        return end();
    }

    Value& operator[](const Key& key) {
        for (auto it = begin(); it != end(); ++it)
            if (it->first == key)
                return it->second;
        insert({key, {}});
        return base.back().second;
    }

    auto begin() {
        return base.begin();
    }
    auto end() {
        return base.end();
    }

    auto begin() const {
        return base.begin();
    }
    auto end() const {
        return base.end();
    }

    size_t size() const {
        return base.size();
    }

  private:
    pstd::vector<pstd::pair<Key, Value>> base;
    Pred pred;
};

}  // namespace pstd

#endif  // PINE_STD_MAP_H