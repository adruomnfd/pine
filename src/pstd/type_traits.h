#ifndef PINE_STD_TYPE_TRAITS_H
#define PINE_STD_TYPE_TRAITS_H

#include <pstd/stdint.h>

namespace pstd {

// integral_constant
template <typename T, T Value>
struct integral_constant {
    using value_type = T;

    static constexpr T value = Value;
};

// true_type
using true_type = integral_constant<bool, true>;

// false_type
using false_type = integral_constant<bool, false>;

// enable_if
template <bool, typename T = void>
struct enable_if {};
template <typename T>
struct enable_if<true, T> {
    using type = T;
};
template <bool Value, typename T = void>
using enable_if_t = typename enable_if<Value, T>::type;

// type_identity
template <typename T>
struct type_identity {
    using type = T;
};
template <typename T>
using type_identity_t = typename type_identity<T>::type;

// conditional
template <bool, typename T, typename U>
struct conditional;
template <typename T, typename U>
struct conditional<false, T, U> {
    using type = T;
};
template <typename T, typename U>
struct conditional<true, T, U> {
    using type = U;
};
template <bool Value, typename T, typename U>
using conditional_t = typename conditional<Value, T, U>::type;

// is_same
template <typename T, typename U>
struct is_same : false_type {};
template <typename T>
struct is_same<T, T> : true_type {};
template <typename T, typename U>
inline constexpr bool is_same_v = is_same<T, U>::value;

// is_array
template <typename T>
struct is_array : false_type {};
template <typename T>
struct is_array<T[]> : true_type {};
template <typename T, int N>
struct is_array<T[N]> : true_type {};
template <typename T>
inline constexpr bool is_array_v = is_array<T>::value;

// is_function
template <typename T>
struct is_function : false_type {};
template <typename R, typename... Args>
struct is_function<R(Args...)> : true_type {};
template <typename T>
inline constexpr bool is_function_v = is_function<T>::value;

// is_integral
template <typename T>
struct is_integral : false_type {};
template <>
struct is_integral<int8_t> : true_type {};
template <>
struct is_integral<int16_t> : true_type {};
template <>
struct is_integral<int32_t> : true_type {};
template <>
struct is_integral<int64_t> : true_type {};
template <>
struct is_integral<uint8_t> : true_type {};
template <>
struct is_integral<uint16_t> : true_type {};
template <>
struct is_integral<uint32_t> : true_type {};
template <>
struct is_integral<uint64_t> : true_type {};
template <typename T>
inline constexpr bool is_integral_v = is_integral<T>::value;

// is_floating_point
template <typename T>
struct is_floating_point : false_type {};
template <>
struct is_floating_point<float> : true_type {};
template <>
struct is_floating_point<double> : true_type {};
template <typename T>
inline constexpr bool is_floating_point_v = is_floating_point<T>::value;

// is_arithmetic
template <typename T>
struct is_arithmetic : integral_constant<bool, is_integral_v<T> || is_floating_point_v<T>> {};
template <typename T>
inline constexpr bool is_arithmetic_v = is_arithmetic<T>::value;

// remove_const
template <typename T>
struct remove_const {
    using type = T;
};
template <typename T>
struct remove_const<const T> {
    using type = T;
};
template <typename T>
using remove_const_t = typename remove_const<T>::type;

// remove_volatile
template <typename T>
struct remove_volatile {
    using type = T;
};
template <typename T>
struct remove_volatile<volatile T> {
    using type = T;
};
template <typename T>
using remove_volatile_t = typename remove_volatile<T>::type;

// remove_cv
template <typename T>
struct remove_cv {
    using type = remove_volatile_t<remove_const_t<T>>;
};
template <typename T>
using remove_cv_t = typename remove_cv<T>::type;

// remove_reference
template <typename T>
struct remove_reference {
    using type = T;
};
template <typename T>
struct remove_reference<T &> {
    using type = T;
};
template <typename T>
struct remove_reference<T &&> {
    using type = T;
};
template <typename T>
using remove_reference_t = typename remove_reference<T>::type;

// remove_extent
template <typename T>
struct remove_extent {
    using type = T;
};
template <typename T>
struct remove_extent<T[]> {
    using type = T;
};
template <typename T, size_t N>
struct remove_extent<T[N]> {
    using type = T;
};
template <typename T>
using remove_extent_t = typename remove_extent<T>::type;

// decay
template <typename T, bool IsArray = is_array_v<T>, bool IsFunction = is_function_v<T>>
struct decay_delector;

template <typename T>
struct decay_delector<T, false, false> {
    using type = remove_cv_t<T>;
};

template <typename T>
struct decay_delector<T, true, false> {
    using type = remove_extent_t<T> *;
};

template <typename T>
struct decay_delector<T, false, true> {
    using type = T *;
};

template <typename T>
struct decay {
  private:
    using U = remove_reference_t<T>;

  public:
    using type = typename decay_delector<U>::type;
};
template <typename T>
using decay_t = typename decay<T>::type;

// declval
template <typename T>
T declval() {
    return declval<T>();
}

// is_convertible
template <typename From, typename To>
struct is_convertible {
  private:
    static constexpr true_type test(To);
    static constexpr false_type test(...);

  public:
    static constexpr bool value = decltype(test(pstd::declval<From>()))::value;
};
template <typename From, typename To>
inline constexpr bool is_convertible_v = is_convertible<From, To>::value;

// corresponding_int
template <typename T>
struct corresponding_int;
template <>
struct corresponding_int<float> {
    using type = int32_t;
};
template <>
struct corresponding_int<double> {
    using type = int64_t;
};
template <typename T>
using corresponding_int_t = typename corresponding_int<T>::type;

// corresponding_uint
template <typename T>
struct corresponding_uint;
template <>
struct corresponding_uint<float> {
    using type = uint32_t;
};
template <>
struct corresponding_uint<double> {
    using type = uint64_t;
};
template <typename T>
using corresponding_uint_t = typename corresponding_uint<T>::type;

// is_pointerish
template <typename T>
struct is_pointerish {
    template <typename U>
    static constexpr true_type test(decltype(*pstd::declval<U>()) *);
    template <typename U>
    static constexpr true_type test(...);

    static constexpr bool value = decltype(test<T>(0))::value;
};
template <typename T>
inline constexpr bool is_pointerish_v = is_pointerish<T>::value;

// defered_bool
template <typename T, bool Value>
struct defered_bool {
    static constexpr bool value = Value;
};

// priority_tag
template <int I>
struct priority_tag : priority_tag<I - 1> {};
template <>
struct priority_tag<0> {};

}  // namespace pstd

#endif  // PINE_STD_TYPE_TRAITS_H