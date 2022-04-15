#ifndef PINE_UTIL_REFLECT_H
#define PINE_UTIL_REFLECT_H

#include <core/defines.h>

#include <pstd/vector.h>
#include <pstd/map.h>

namespace pine {

template <typename T>
struct IsVector {
    static constexpr bool value = false;
};
template <typename T>
struct IsVector<pstd::vector<T>> {
    static constexpr bool value = true;
};

template <typename T>
struct IsMap {
    static constexpr bool value = false;
};
template <typename Key, typename Value>
struct IsMap<pstd::map<Key, Value>> {
    static constexpr bool value = true;
};

template <typename T>
struct IsPointer {
    template <typename U>
    static constexpr pstd::true_type Check(pstd::decay_t<decltype(*U())>*);
    template <typename U>
    static constexpr pstd::false_type Check(...);

    static constexpr bool value = decltype(Check<T>(0))::value;
};

template <typename T>
struct IsIterable {
    template <typename U>
    static constexpr pstd::true_type Check(decltype(pstd::begin(U()))*, decltype(pstd::end(U()))*);
    template <typename U>
    static constexpr pstd::false_type Check(...);

    static constexpr bool value = decltype(Check<T>(0, 0))::value;
};

template <typename F, typename... Args,
          typename = decltype((*(pstd::decay_t<F>*)0)((*(pstd::decay_t<Args>*)0)...))>
decltype(auto) InvokeImpl(pstd::priority_tag<2>, F&& f, Args&&... args) {
    return f(pstd::forward<Args>(args)...);
}

template <typename F, typename... Args,
          typename = decltype((*(pstd::decay_t<F>*)0)(pstd::move(*(pstd::decay_t<Args>*)0)...))>
decltype(auto) InvokeImpl(pstd::priority_tag<1>, F&& f, Args&&... args) {
    return f(pstd::forward<Args>(args)...);
}

template <typename F, typename First, typename... Rest>
decltype(auto) InvokeImpl(pstd::priority_tag<0>, F&& f, First&&, Rest&&... rest) {
    return InvokeImpl(pstd::priority_tag<2>{}, pstd::forward<F>(f), pstd::forward<Rest>(rest)...);
}

template <typename F, typename... Args>
decltype(auto) Invoke(F&& f, Args&&... args) {
    return InvokeImpl(pstd::priority_tag<2>{}, pstd::forward<F>(f), pstd::forward<Args>(args)...);
}

}  // namespace pine

#endif  // PINE_UTIL_REFLECT_H