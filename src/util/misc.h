#ifndef PINE_UTIL_MISC_H
#define PINE_UTIL_MISC_H

#include <core/defines.h>

#include <pstd/optional.h>

namespace pine {

template <typename T, typename Key>
auto Find(const T& x, const Key& key) {
    auto it = x.find(key);
    using Value = decltype(it->second);

    if (it != x.end())
        return pstd::optional<Value>(it->second);
    else
        return pstd::optional<Value>(pstd::nullopt);
}

}  // namespace pine

#endif  // PINE_UTIL_MISC_H