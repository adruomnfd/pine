#ifndef PINE_UTIL_MISC_H
#define PINE_UTIL_MISC_H

#include <core/defines.h>

#include <optional>

namespace pine {

template <typename T, typename Key>
auto Find(const T& x, const Key& key) {
    auto it = x.find(key);
    using Value = decltype(it->second);

    if (it != x.end())
        return std::optional<Value>(it->second);
    else
        return std::optional<Value>(std::nullopt);
}

}  // namespace pine

#endif  // PINE_UTIL_MISC_H