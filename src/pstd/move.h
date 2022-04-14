#ifndef PINE_STD_MOVE_H
#define PINE_STD_MOVE_H

namespace pstd {

template <typename T>
T&& move(T& x) {
    return static_cast<T&&>(x);
}

template <typename T>
T&& forward(T& x) {
    return static_cast<T&&>(x);
}

template <typename T>
void swap(T& x, T& y) {
    T temp = move(x);
    x = move(y);
    y = move(temp);
}

}  // namespace pstd

#endif  // PINE_STD_MOVE_H