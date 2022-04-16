#ifndef PINE_STD_ALGORITHM_H
#define PINE_STD_ALGORITHM_H

#include <pstd/move.h>
#include <pstd/type_traits.h>

namespace pstd {

template <typename T, typename = decltype(pstd::declval<T>().begin())>
inline auto begin(T&& x) {
    return x.begin();
}

template <typename T, typename = decltype(pstd::declval<T>().end())>
inline auto end(T&& x) {
    return x.end();
}

template <typename T, typename = decltype(pstd::declval<T>().size())>
inline auto size(T&& x) {
    return x.size();
}

template <typename It, typename = decltype(It() - It())>
inline auto distance(It first, It last, pstd::priority_tag<1>) {
    return last - first;
}

template <typename It, typename = void>
inline auto distance(It first, It last, pstd::priority_tag<0>) {
    ptrdiff_t dist = 0;
    for (; first != last; ++first)
        ++dist;
    return dist;
}

template <typename It>
inline auto distance(It first, It last) {
    return pstd::distance(first, last, pstd::priority_tag<1>{});
}

template <typename InputIt, typename OutputIt>
inline void copy(InputIt first, InputIt last, OutputIt d_first) {
    for (; first != last; ++first, ++d_first)
        *d_first = *first;
}

template <typename InputIt, typename OutputIt>
inline void move(InputIt first, InputIt last, OutputIt d_first) {
    for (; first != last; ++first, ++d_first)
        *d_first = pstd::move(*first);
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

            pstd::swap(*prev, *i);
            if (pivot != i)
                pstd::swap(*pivot, *i);
        }
    }

    pstd::sort(first, pivot, pstd::forward<Pred>(pred));
    ++pivot;
    pstd::sort(pivot, last, pstd::forward<Pred>(pred));
}

template <typename It, typename T>
inline It find_first_of(It first, It last, const T& value) {
    for (; first != last; ++first)
        if (*first == value)
            return first;
    return last;
}

template <typename It, typename T>
inline It find_last_of(It first, It last, const T& value) {
    for (; first != last; --last)
        if (*last == value)
            return last;
    return last;
}

template <typename It, typename T>
inline void replace(It first, It last, const T& old, const T& new_) {
    for (; first != last; ++first)
        if (*first == old)
            *first = new_;
}

template <typename It, typename T>
inline size_t count(It first, It last, const T& value) {
    size_t c = 0;
    for (; first != last; ++first)
        if (*first == value)
            ++c;
    return c;
}

template <typename T>
inline T trim(const T& x, size_t first, size_t size) {
    return T(pstd::begin(x) + first, pstd::begin(x) + first + size);
}

template <typename T>
inline T trim(const T& x, size_t first) {
    return T(pstd::begin(x) + first, pstd::end(x));
}

template <typename T, typename It>
inline T erase(const T& x, It first, It last) {
    T y = T(pstd::size(x) + pstd::distance(first, last));
    auto i = pstd::begin(y);

    for (auto it = pstd::begin(x); it != first; ++it)
        *(i++) = *it;
    for (auto it = last; it != pstd::end(x); ++it)
        *(i++) = *it;

    return y;
}

template <typename It, typename F>
It remove_if(It first, It last, F&& f) {
    It tail = first;
    for (; first != last; ++first)
        if (first != tail && !f(*first))
            *(tail++) = *first;

    return tail;
}

}  // namespace pstd

#endif  // PINE_STD_ALGORITHM_H