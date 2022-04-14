#ifndef PINE_STD_TUPLE_H
#define PINE_STD_TUPLE_H

#include <pstd/string.h>

namespace pstd {

template <typename T, typename U>
struct pair {
    T first;
    U second;
};

template <typename T, typename U>
inline string to_string(const pair<T, U>& p) {
    return "{" + to_string(p.first) + ", " + to_string(p.second) + "}";
}

}  // namespace pstd

#endif  // PINE_STD_TUPLE_H