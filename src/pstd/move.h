#ifndef PINE_STD_MOVE_H
#define PINE_STD_MOVE_H

#include <pstd/type_traits.h>

namespace pstd {

template <typename T>
inline T&& move(T& x) {
    return static_cast<T&&>(x);
}

template <typename T>
inline T&& forward(type_identity_t<T>& x) {
    return static_cast<T&&>(x);
}

template <typename T>
inline void swap(T& x, T& y) {
    T temp = pstd::move(x);
    x = pstd::move(y);
    y = pstd::move(temp);
}

template <typename T, typename U>
inline T exchange(T& x, U&& newval) {
    T old = pstd::move(x);
    x = pstd::forward<T>(newval);
    return old;
}

template <typename To, typename From>
inline To bitcast(const From& x) {
    // TODO
    return *(To*)&x;
}

}  // namespace pstd

#endif  // PINE_STD_MOVE_H