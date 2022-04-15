#ifndef PINE_STD_MOVE_H
#define PINE_STD_MOVE_H

namespace pstd {

template <typename T>
inline T&& move(T& x) {
    return static_cast<T&&>(x);
}

template <typename T>
inline T&& forward(T& x) {
    return static_cast<T&&>(x);
}

template <typename T>
inline void swap(T& x, T& y) {
    T temp = move(x);
    x = move(y);
    y = move(temp);
}

template <typename T, typename U>
inline T exchange(T& x, U&& newval) {
    T old = move(x);
    x = forward(newval);
    return old;
}

}  // namespace pstd

#endif  // PINE_STD_MOVE_H