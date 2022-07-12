#ifndef PINE_STD_TUPLE_H
#define PINE_STD_TUPLE_H

#include <pstd/archive.h>

namespace pstd {

template <typename T, typename U>
struct pair {
    PSTD_ARCHIVE(first, second)

    T first;
    U second;
};

template <typename T, typename... Ts>
struct tuple {
    PSTD_ARCHIVE(value, rest)

    T value;
    tuple<Ts...> rest;
};

template <typename T>
struct tuple<T> {
    PSTD_ARCHIVE(value)

    T value;
};

}  // namespace pstd

#endif  // PINE_STD_TUPLE_H