#ifndef PINE_STD_ALGORITHM_H
#define PINE_STD_ALGORITHM_H

#include <pstd/stdint.h>
#include <pstd/move.h>

namespace pstd {

template <typename T>
inline auto begin(T&& x) {
    return x.begin();
}

template <typename T>
inline auto end(T&& x) {
    return x.end();
}

template <typename T>
inline auto size(T&& x) {
    return x.size();
}

template <typename InputIt, typename OutputIt>
inline void copy(InputIt first, InputIt last, OutputIt d_first) {
    for (; first != last; ++first, ++d_first)
        *d_first = *first;
}

template <typename InputIt, typename T>
inline void fill(InputIt first, InputIt last, const T& value) {
    for (; first != last; ++first)
        *first = value;
}

template <typename T>
struct less {
    bool operator()(const T& l, const T& r) const {
        return l < r;
    }
};

template <typename It, typename Pred>
inline It lower_bound(It first, It last, Pred&& pred) {
    while (first != last) {
        It mid = first + (last - first) / 2;

        if (pred(*mid))
            first = mid + 1;
        else
            last = mid;
    }

    return last;
}

template <typename It, typename Pred>
inline void sort(It first, It last, Pred&& pred) {
    if (first == last)
        return;
    It pivot = first;

    It i = first;
    ++i;
    if (i == last)
        return;

    for (; i != last; ++i) {
        if (pred(*i, *pivot)) {
            It prev = pivot;
            ++pivot;

            swap(*prev, *i);
            if (pivot != i)
                swap(*pivot, *i);
        }
    }

    sort(first, pivot, forward<Pred>(pred));
    ++pivot;
    sort(pivot, last, forward<Pred>(pred));
}

}  // namespace pstd

#endif  // PINE_STD_ALGORITHM_H