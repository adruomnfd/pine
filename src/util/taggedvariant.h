#ifndef PINE_UTIL_TAGGEDVARIANT_H
#define PINE_UTIL_TAGGEDVARIANT_H

#include <core/defines.h>
#include <util/log.h>

#include <pstd/type_traits.h>
#include <pstd/move.h>
#include <pstd/new.h>

namespace pine {

namespace detail {

template <typename T, typename... Ts>
struct Union {
    using First = T;
    using Rest = Union<Ts...>;

    Union() {
    }
    ~Union() {
    }
    Union(const Union&) = delete;
    Union(Union&&) = delete;
    Union& operator=(Union&) = delete;
    Union& operator=(Union&&) = delete;

    template <typename X>
    void Assign(X&& v) {
        using Xy = pstd::decay_t<X>;

        if constexpr (pstd::is_same<Xy, First>::value)
            new (&first) Xy(pstd::forward<X>(v));
        else
            rest.Assign(pstd::forward<X>(v));
    }

    template <typename X>
    X& Be() {
        if constexpr (pstd::is_same<X, First>::value)
            return first;
        else
            return rest.template Be<X>();
    }
    template <typename X>
    const X& Be() const {
        if constexpr (pstd::is_same<X, First>::value)
            return first;
        else
            return rest.template Be<X>();
    }

    union {
        First first;
        Rest rest;
    };
    static constexpr bool isFinal = false;
};

template <typename T>
struct Union<T> {
    using First = T;

    Union() {
    }
    ~Union() {
    }
    Union(const Union&) = delete;
    Union(Union&&) = delete;
    Union& operator=(Union&) = delete;
    Union& operator=(Union&&) = delete;

    template <typename X>
    void Assign(X&& v) {
        using Xy = pstd::decay_t<X>;
        static_assert(pstd::is_same<Xy, First>::value, "type X is not one of the Union's");

        new (&first) Xy(pstd::forward<X>(v));
    }

    template <typename X>
    X& Be() {
        static_assert(pstd::is_same<X, First>::value, "type X is not one of the Union's");

        return first;
    }
    template <typename X>
    const X& Be() const {
        static_assert(pstd::is_same<X, First>::value, "type X is not one of the Union's");

        return first;
    }

    union {
        First first;
    };
    static constexpr bool isFinal = true;
};

}  // namespace detail

template <typename T>
struct HasOpEq {
    template <typename U>
    static constexpr pstd::true_type Check(decltype(U() == U())*);
    template <typename U>
    static constexpr pstd::false_type Check(...);

    static constexpr bool value = decltype(Check<T>(0))::value;
};

template <typename T, typename First, typename... Rest>
constexpr int Index() {
    static_assert(pstd::is_same<T, First>::value || sizeof...(Rest) != 0,
                  "type T is not in the parameter pack");

    if constexpr (pstd::is_same<T, First>::value)
        return 0;
    else
        return 1 + Index<T, Rest...>();
}

template <typename T, typename... Args>
struct CommonType {
    using type = T;
};

template <typename F, typename U, typename T>
constexpr decltype(auto) FuncWrapper(F&& f, U&& x) {
    return f(pstd::forward<U>(x).template Be<T>());
}

template <typename... Ts, typename F, typename U>
decltype(auto) DispatchJumpTable(F&& f, int index, U&& value) {
    using T = typename CommonType<Ts...>::type;
    using Fwt = pstd::decay_t<decltype(FuncWrapper<F, U, T>)>;
    constexpr static Fwt table[] = {FuncWrapper<F, U, Ts>...};

    return table[index](pstd::forward<F>(f), pstd::forward<U>(value));
}

template <typename... Ts, typename F, typename T>
decltype(auto) DispatchIfElse(F&& f, int index, T&& value) {
    using Ty = pstd::decay_t<T>;

    if constexpr (Ty::isFinal)
        return f(pstd::forward<T>(value).first);
    else if (index == 0)
        return f(pstd::forward<T>(value).first);
    else
        return Dispatch(pstd::forward<F>(f), index - 1, pstd::forward<T>(value).rest);
}

template <typename... Ts, typename F, typename T>
decltype(auto) Dispatch(F&& f, int index, T&& value) {
    return DispatchJumpTable<Ts...>(pstd::forward<F>(f), index, pstd::forward<T>(value));
}

template <typename... Ts>
struct TaggedVariant {
    using Aggregate = detail::Union<Ts...>;

    TaggedVariant() = default;
    TaggedVariant(const TaggedVariant& rhs) {
        rhs.TryDispatch([&](const auto& x) { value.Assign(x); });
        tag = rhs.tag;
    }
    TaggedVariant(TaggedVariant&& rhs) {
        rhs.TryDispatch([&](auto&& x) { value.Assign(pstd::move(x)); });
        tag = pstd::exchange(rhs.tag, tag);
    }
    TaggedVariant& operator=(TaggedVariant rhs) {
        rhs.TryDispatch([&](auto& rx) {
            if (IsValid()) {
                auto oldRx = pstd::move(rx);
                Dispatch([&](auto& lx) { rhs.value.Assign(pstd::move(lx)); });
                value.Assign(pstd::move(oldRx));
            } else {
                value.Assign(pstd::move(rx));
            }
        });

        pstd::swap(tag, rhs.tag);
        return *this;
    }
    ~TaggedVariant() {
        TryDispatch([](auto& x) {
            using T = pstd::decay_t<decltype(x)>;
            x.~T();
        });
    }

    template <typename T>
    TaggedVariant(T&& v) {
        value.Assign(pstd::forward<T>(v));
        tag = pine::Index<pstd::decay_t<T>, Ts...>();
    }

    template <typename T>
    TaggedVariant& operator=(T&& v) {
        this->~TaggedVariant();
        value.Assign(pstd::forward<T>(v));
        tag = Index<pstd::decay_t<T>, Ts...>();
        return *this;
    }

    template <typename F>
    decltype(auto) Dispatch(F&& f) {
        return pine::Dispatch<Ts...>(pstd::forward<F>(f), tag, value);
    }

    template <typename F>
    decltype(auto) Dispatch(F&& f) const {
        return pine::Dispatch<Ts...>(pstd::forward<F>(f), tag, value);
    }

    template <typename F>
    void TryDispatch(F&& f) {
        if (IsValid())
            pine::Dispatch<Ts...>(pstd::forward<F>(f), tag, value);
    }

    template <typename F>
    void TryDispatch(F&& f) const {
        if (IsValid())
            pine::Dispatch<Ts...>(pstd::forward<F>(f), tag, value);
    }

    bool IsValid() const {
        return tag != (uint8_t)-1;
    }

    int Tag() const {
        return tag;
    }

    template <typename T>
    static int Index() {
        return pine::Index<T, Ts...>();
    }

    template <typename T>
    bool Is() const {
        return tag == Index<T>();
    }

    template <typename T>
    T& Be() {
        return value.template Be<T>();
    }

    template <typename T>
    const T& Be() const {
        return value.template Be<T>();
    }

    PSTD_ARCHIVE(value, tag)

  private:
    Aggregate value;
    uint8_t tag = (uint8_t)-1;
};

}  // namespace pine

#endif  // PINE_UTIL_TAGGEDVARIANT_H