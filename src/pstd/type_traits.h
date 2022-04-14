#ifndef PINE_STD_TYPE_TRAITS_H
#define PINE_STD_TYPE_TRAITS_H

#include <pstd/stddef.h>

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
struct remove_reference<T&> {
    using type = T;
};
template <typename T>
struct remove_reference<T&&> {
    using type = T;
};
template <typename T>
using remove_reference_t = typename remove_reference<T>::type;

// decay
template <typename T>
struct decay {
    using type = remove_reference_t<remove_cv_t<T>>;
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
    static constexpr bool value = decltype(test(declval<From>()))::value;
};
template <typename From, typename To>
inline constexpr bool is_convertible_v = is_convertible<From, To>::value;

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

};  // namespace pstd

#endif  // PINE_STD_TYPE_TRAITS_H