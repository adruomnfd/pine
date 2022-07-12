#ifndef PINE_STD_FIELD_H
#define PINE_STD_FIELD_H

#include <pstd/move.h>
#include <pstd/type_traits.h>

#define PSTD_ARCHIVE(...)                                 \
    template <typename Archiver>                          \
    decltype(auto) _archive_(Archiver&& archiver) {       \
        return archiver(__VA_ARGS__);                     \
    }                                                     \
    template <typename Archiver>                          \
    decltype(auto) _archive_(Archiver&& archiver) const { \
        return archiver(__VA_ARGS__);                     \
    }

namespace pstd {

template <typename T>
struct has_archive_method {
    struct pseudo_archiver {
        template <typename... Ts>
        void operator()(Ts&&...) const {
        }
    };

    template <typename U>
    static constexpr true_type test(decltype(pstd::declval<U>()._archive_(pseudo_archiver{}))*);
    template <typename U>
    static constexpr false_type test(...);

    static constexpr bool value = decltype(test<T>(0))::value;
};
template <typename T>
inline constexpr bool has_archive_method_v = has_archive_method<T>::value;

template <typename T, typename F>
constexpr decltype(auto) apply_fields(T&& x, F&& f) {
    return x._archive_(pstd::forward<F>(f));
}

template <typename F>
struct foreach_field_helper {
    template <typename... Ts>
    void operator()(Ts&&... ts) {
        apply(forward<Ts>(ts)...);
    }

    template <typename T, typename... Ts>
    constexpr void apply(T&& x, Ts&&... rest) {
        f(forward<T>(x));
        if constexpr (sizeof...(rest) != 0)
            apply(forward<Ts>(rest)...);
    }

    F&& f;
};

template <typename T, typename F>
constexpr void foreach_field(T&& x, F&& f) {
    foreach_field_helper<F> helper{pstd::forward<F>(f)};
    x._archive_(helper);
}

}  // namespace pstd

#endif  // PINE_STD_FIELD_H