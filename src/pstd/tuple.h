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
struct tuple_base {
    PSTD_ARCHIVE(value, rest)

    T value;
    tuple_base<Ts...> rest;
};

template <typename T>
struct tuple_base<T> {
    PSTD_ARCHIVE(value)

    T value;
};

template <typename... Ts>
struct tuple : tuple_base<Ts...> {};

}  // namespace pstd

#endif  // PINE_STD_TUPLE_H