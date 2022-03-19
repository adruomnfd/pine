#ifndef PINE_UTIL_REFLECT_H
#define PINE_UTIL_REFLECT_H

#include <core/defines.h>

#include <tuple>
#include <iterator>

namespace pine {

template <typename T, bool Value>
struct DeferredBool {
    static constexpr bool value = Value;
};

template <int I>
struct PriorityTag : PriorityTag<I - 1> {};

template <>
struct PriorityTag<0> {};

template <typename T>
struct IsVector {
    template <typename U>
    static constexpr std::true_type Check(decltype((void(U::*)(size_t)) & U::resize)*);
    template <typename U>
    static constexpr std::false_type Check(...);

    static constexpr bool value = decltype(Check<T>(0))::value;
};

template <typename T>
struct IsMap {
    template <typename U>
    static constexpr std::true_type Check(typename U::key_type*);
    template <typename U>
    static constexpr std::false_type Check(...);

    static constexpr bool value = decltype(Check<T>(0))::value;
};

template <typename T>
struct IsPointer {
    template <typename U>
    static constexpr std::true_type Check(std::decay_t<decltype(*U())>*);
    template <typename U>
    static constexpr std::false_type Check(...);

    static constexpr bool value = decltype(Check<T>(0))::value;
};

template <typename T>
struct IsIterable {
    template <typename U>
    static constexpr std::true_type Check(decltype(std::begin(U()))*, decltype(std::begin(U()))*);
    template <typename U>
    static constexpr std::false_type Check(...);

    static constexpr bool value = decltype(Check<T>(0, 0))::value;
};

template <typename T>
struct HasArchiveMethod {
    struct Invokable {
        template <typename... Ts>
        void operator()(Ts&&...) {
        }
    };
    template <typename U>
    static constexpr std::true_type Check(
        decltype((void(U::*)(Invokable&)) & U::template Archive<Invokable&>)*,
        decltype((void(U::*)(Invokable&) const) & U::template Archive<Invokable&>)*);
    template <typename U>
    static constexpr std::false_type Check(...);

    static constexpr bool value = decltype(Check<T>(0, 0))::value;
};

template <typename T, size_t I>
struct ToAny {
    template <typename U, typename = typename std::enable_if_t<!std::is_same<T, U>::value>>
    constexpr operator U() const noexcept {
        return U(*this);
    }
};

template <typename T, typename... Ts>
struct IsAggregateInitializableFrom {
    template <typename U>
    static constexpr std::true_type Check(decltype(U{Ts()...}) *);
    template <typename U>
    static constexpr std::false_type Check(...);

    static constexpr bool value = decltype(Check<T>(0))::value;
};

template <typename T, size_t... Is>
constexpr size_t NumFieldsImpl(std::index_sequence<Is...>) {
    if constexpr (IsAggregateInitializableFrom<T, ToAny<T, Is>...>::value)
        return sizeof...(Is);
    else
        return NumFieldsImpl<T>(std::make_index_sequence<sizeof...(Is) - 1>());
}
template <typename Ty, typename = typename std::enable_if_t<std::is_pod<std::decay_t<Ty>>::value>>
constexpr size_t NumFields() {
    using T = std::decay_t<Ty>;
    return NumFieldsImpl<T>(std::make_index_sequence<sizeof(T) / sizeof(char)>());
}

template <size_t I>
struct SizeTag {};

template <class T>
constexpr auto TieAsTuple(T&&, SizeTag<0>) {
    return std::false_type{};
}
template <class T>
constexpr auto TieAsTuple(T&& x, SizeTag<1>) {
    auto&& [a] = x;
    return std::tie(a);
}
template <class T>
constexpr auto TieAsTuple(T&& x, SizeTag<2>) {
    auto&& [a, b] = x;
    return std::tie(a, b);
}
template <class T>
constexpr auto TieAsTuple(T&& x, SizeTag<3>) {
    auto&& [a, b, c] = x;
    return std::tie(a, b, c);
}
template <class T>
constexpr auto TieAsTuple(T&& x, SizeTag<4>) {
    auto&& [a, b, c, d] = x;
    return std::tie(a, b, c, d);
}
template <class T>
constexpr auto TieAsTuple(T&& x, SizeTag<5>) {
    auto&& [a, b, c, d, e] = x;
    return std::tie(a, b, c, d, e);
}
template <class T>
constexpr auto TieAsTuple(T&& x, SizeTag<6>) {
    auto&& [a, b, c, d, e, f] = x;
    return std::tie(a, b, c, d, e, f);
}
template <class T>
constexpr auto TieAsTuple(T&& x, SizeTag<7>) {
    auto&& [a, b, c, d, e, f, g] = x;
    return std::tie(a, b, c, d, e, f, g);
}
template <class T>
constexpr auto TieAsTuple(T&& x, SizeTag<8>) {
    auto&& [a, b, c, d, e, f, g, h] = x;
    return std::tie(a, b, c, d, e, f, g, h);
}

template <class T>
constexpr auto TieAsTuple(T&& x) {
    return TieAsTuple(std::forward<T>(x), SizeTag<NumFields<T>()>{});
}

template <size_t I, typename T>
constexpr auto&& Get(T&& x) {
    return std::get<I>(TieAsTuple(std::forward<T>(x)));
}

template <typename T, typename F, int Index = 0,
          typename = typename std::enable_if_t<std::is_pod<std::decay_t<T>>::value>>
constexpr void ForEachField(T&& x, F&& f) {
    if constexpr (Index != NumFields<T>()) {
        f(Get<Index>(x));
        ForEachField<T, F, Index + 1>(std::forward<T>(x), std::forward<F>(f));
    }
}

template <typename F>
struct ForEachFieldHelper {
    template <typename F2>
    ForEachFieldHelper(F2&& f) : f(std::forward<F2>(f)) {
    }

    template <typename... Ts>
    void operator()(Ts&&... ts) {
        Apply(std::forward<Ts>(ts)...);
    }

    template <typename T, typename... Ts>
    constexpr void Apply(T&& x, Ts&&... rest) {
        f(std::forward<T>(x));
        if constexpr (sizeof...(rest) != 0)
            Apply(std::forward<Ts>(rest)...);
    }

    F f;
};

template <typename T, typename F,
          typename = typename std::enable_if_t<HasArchiveMethod<std::decay_t<T>>::value>>
constexpr void ForEachField(T&& x, F&& f) {
    ForEachFieldHelper<F> helper(std::forward<F>(f));
    x.Archive(helper);
}

template <typename T>
struct IsDecomposable {
    template <typename U>
    static constexpr std::true_type Check(
        std::enable_if_t<std::is_pod<U>::value || HasArchiveMethod<U>::value>*);
    template <typename U>
    static constexpr std::false_type Check(...);

    static constexpr bool value = decltype(Check<T>(0))::value;
};

}  // namespace pine

#endif  // PINE_UTIL_REFLECT_H