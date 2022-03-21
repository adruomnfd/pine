#ifndef PINE_UTIL_TAGGEDVARIANT_H
#define PINE_UTIL_TAGGEDVARIANT_H

#include <core/defines.h>
#include <util/log.h>

#include <cstdint>
#include <utility>

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
        using Xy = std::decay_t<X>;

        if constexpr (std::is_same<Xy, First>::value)
            new (&first) Xy(std::forward<X>(v));
        else
            rest.Assign(std::forward<X>(v));
    }

    template <typename X>
    X& Be() {
        if constexpr (std::is_same<X, First>::value)
            return first;
        else
            return rest.template Be<X>();
    }
    template <typename X>
    const X& Be() const {
        if constexpr (std::is_same<X, First>::value)
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
        using Xy = std::decay_t<X>;
        static_assert(std::is_same<Xy, First>::value, "type X is not one of the Union's");

        new (&first) Xy(std::forward<X>(v));
    }

    template <typename X>
    X& Be() {
        static_assert(std::is_same<X, First>::value, "type X is not one of the Union's");

        return first;
    }
    template <typename X>
    const X& Be() const {
        static_assert(std::is_same<X, First>::value, "type X is not one of the Union's");

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
    static constexpr std::true_type Check(decltype(U() == U())*);
    template <typename U>
    static constexpr std::false_type Check(...);

    static constexpr bool value = decltype(Check<T>(0))::value;
};

template <typename T, typename U>
struct CopyQualifier {
    using type = U;
};
template <typename T, typename U>
struct CopyQualifier<const T, U> {
    using type = const U;
};
template <typename T, typename U>
struct CopyQualifier<T&, U> {
    using type = U&;
};
template <typename T, typename U>
struct CopyQualifier<const T&, U> {
    using type = const U&;
};
template <typename T, typename U>
using CopyQualifier_t = typename CopyQualifier<T, U>::type;

template <typename T, typename First, typename... Rest>
constexpr int Index() {
    static_assert(std::is_same<T, First>::value || sizeof...(Rest) != 0,
                  "type T is not in the parameter pack");

    if constexpr (std::is_same<T, First>::value)
        return 0;
    else
        return 1 + Index<T, Rest...>();
}

template <typename F, typename T>
decltype(auto) Dispatch(F&& f, int index, T&& value) {
    using Ty = std::decay_t<T>;

    if constexpr (Ty::isFinal) {
        CHECK_EQ(index, 0);
        return f(std::forward<CopyQualifier_t<T, typename Ty::First>>(value.first));
    } else {
        if (index == 0)
            return f(std::forward<CopyQualifier_t<T, typename Ty::First>>(value.first));
        else
            return Dispatch(std::forward<F>(f), index - 1,
                            std::forward<CopyQualifier_t<T, typename Ty::Rest>>(value.rest));
    }
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
        rhs.TryDispatch([&](auto&& x) { value.Assign(std::move(x)); });
        tag = std::exchange(rhs.tag, tag);
    }
    TaggedVariant& operator=(TaggedVariant rhs) {
        rhs.TryDispatch([&](auto& rx) {
            if (IsValid()) {
                auto oldRx = std::move(rx);
                Dispatch([&](auto& lx) { rhs.value.Assign(std::move(lx)); });
                value.Assign(std::move(oldRx));
            } else {
                value.Assign(std::move(rx));
            }
        });

        std::swap(tag, rhs.tag);
        return *this;
    }
    ~TaggedVariant() {
        TryDispatch([](auto& x) {
            using T = std::decay_t<decltype(x)>;
            x.~T();
        });
    }

    template <typename T>
    TaggedVariant(T&& v) {
        value.Assign(std::forward<T>(v));
        tag = pine::Index<std::decay_t<T>, Ts...>();
    }
    template <typename T>
    TaggedVariant& operator=(T&& v) {
        this->~TaggedVariant();
        value.Assign(std::forward<T>(v));
        tag = Index<std::decay_t<T>, Ts...>();
        return *this;
    }

    friend bool operator==(const TaggedVariant& a, const TaggedVariant& b) {
        if (a.Tag() != b.Tag())
            return false;
        return a.Dispatch([&](const auto& ax) {
            return b.Dispatch([&](const auto& bx) {
                if constexpr (std::is_same<std::decay_t<decltype(ax)>,
                                           std::decay_t<decltype(ax)>>::value) {
                    if constexpr (HasOpEq<std::decay_t<decltype(ax)>>::value) {
                        return ax == bx;
                    } else {
                        LOG_FATAL("Try to compare type that didn't define operator==");
                        return false;
                    }
                } else {
                    return false;
                }
            });
        });
    }
    friend bool operator!=(const TaggedVariant& a, const TaggedVariant& b) {
        return !(a == b);
    }

    template <typename F>
    decltype(auto) Dispatch(F&& f) {
        CHECK(IsValid());
        return pine::Dispatch(std::forward<F>(f), tag, value);
    }

    template <typename F>
    decltype(auto) Dispatch(F&& f) const {
        CHECK(IsValid());
        return pine::Dispatch(std::forward<F>(f), tag, value);
    }

    template <typename F>
    void TryDispatch(F&& f) {
        if (IsValid())
            pine::Dispatch(std::forward<F>(f), tag, value);
    }

    template <typename F>
    void TryDispatch(F&& f) const {
        if (IsValid())
            pine::Dispatch(std::forward<F>(f), tag, value);
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
    bool Is() const{
        return tag == Index<T>();
    }

    template <typename T>
    T& Be() {
        CHECK(IsValid());
        return value.template Be<T>();
    }

    template <typename T>
    const T& Be() const {
        CHECK(IsValid());
        return value.template Be<T>();
    }

    PINE_ARCHIVE(value, tag)

  private:
    Aggregate value;
    uint8_t tag = (uint8_t)-1;
};

}  // namespace pine

#endif  // PINE_UTIL_TAGGEDVARIANT_H