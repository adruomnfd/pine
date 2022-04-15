#ifndef PINE_STD_STRING_H
#define PINE_STD_STRING_H

#include <pstd/math.h>
#include <pstd/vector.h>
#include <pstd/archive.h>
#include <pstd/type_traits.h>

namespace pstd {

inline size_t strlen(const char* str) {
    size_t len = 0;
    while (*(str++))
        ++len;
    return len;
}

inline bool strcmp(const char* lhs, const char* rhs) {
    if (!lhs || !rhs)
        return !lhs && !rhs;

    for (size_t i = 0;; ++i) {
        if (lhs[i] != rhs[i])
            return false;
        if (!lhs[i] || !rhs[i])
            return !lhs[i] && !rhs[i];
    }
}

template <typename T>
struct string_allocator {
    T* alloc(size_t size) const {
        return ::new T[size + 1]();
    }
    void free(T* ptr) const {
        ::delete[] ptr;
    }

    template <typename... Args>
    void construct_at(T* ptr, Args&&... args) const {
        pstd::construct_at(ptr, pstd::forward<Args>(args)...);
    }

    void destruct_at(T* ptr) const {
        *ptr = {};
    }
};

class string : public vector_base<char, string_allocator<char>> {
  public:
    using base = vector_base<char, string_allocator<char>>;
    using base::base;

    string(const char* cstr) : base(pstd::strlen(cstr)) {
        pstd::copy(cstr, cstr + len, begin());
    }

    string(const char* cstr, size_t len) : base(len) {
        pstd::copy(cstr, cstr + len, begin());
    }

    string& operator=(const char* str) {
        resize(pstd::strlen(str));
        pstd::copy(str, str + size(), begin());
        return *this;
    }
    string& operator=(class string_view str);

    string substr(size_t pos) const {
        return string(data() + pos, len - pos);
    }
    string substr(size_t pos, size_t len) const {
        return string(data() + pos, len);
    }

    size_t find_first_of(char c) const {
        size_t i = 0;
        for (; i < size(); ++i)
            if (ptr[i] == c)
                break;
        if (i == size())
            return npos;

        return i;
    }
    size_t find_last_of(char c) const {
        size_t i = 0, last = npos;
        for (; i < size(); ++i)
            if (ptr[i] == c)
                last = i;

        return last;
    }

    string& operator+=(const string& rhs) {
        size_t oldlen = len;

        resize(len + rhs.len);
        pstd::copy(pstd::begin(rhs), pstd::end(rhs), begin() + oldlen);
        return *this;
    }
    string& operator+=(class string_view rhs);
    string& operator+=(const char* rhs);

    friend string operator+(string lhs, const string& rhs) {
        return lhs += rhs;
    }
    friend string operator+(string lhs, const char* rhs) {
        return lhs += rhs;
    }

    const char* c_str() const {
        return data();
    }

    friend bool operator==(const string& lhs, const string& rhs) {
        return strcmp(lhs.data(), rhs.data());
    }
    friend bool operator!=(const string& lhs, const string& rhs) {
        return !strcmp(lhs.data(), rhs.data());
    }
    friend bool operator<(const string& lhs, const string& rhs) {
        return lhs.data() < rhs.data();
    }
    friend bool operator>(const string& lhs, const string& rhs) {
        return lhs.data() > rhs.data();
    }

    inline static const size_t npos = (size_t)-1;
};

class string_view {
  public:
    using iterator = const char*;

    string_view() = default;
    string_view(const char* str) : str(str), len(pstd::strlen(str)) {
    }
    string_view(const char* str, size_t len) : str(str), len(len) {
    }
    string_view(const string& str) : str(str.c_str()), len(pstd::size(str)) {
    }

    string_view substr(size_t pos) const {
        return string_view(str + pos, len - pos);
    }
    string_view substr(size_t pos, size_t len) const {
        return string_view(str + pos, len);
    }

    size_t find_first_of(char c) const {
        size_t i = 0;
        for (; i < size(); ++i)
            if (str[i] == c)
                break;
        if (i == size())
            return npos;

        return i;
    }
    size_t find_last_of(char c) const {
        size_t i = 0, last = npos;
        for (; i < size(); ++i)
            if (str[i] == c)
                last = i;

        return last;
    }

    explicit operator string() const {
        return string(str, len);
    }

    char operator[](size_t i) const {
        return str[i];
    }

    const char* begin() const {
        return str;
    }
    const char* end() const {
        return str + len;
    }

    size_t size() const {
        return len;
    }

    friend bool operator==(string_view lhs, string_view rhs) {
        return pstd::strcmp(lhs.str, rhs.str);
    }
    friend bool operator!=(string_view lhs, string_view rhs) {
        return !pstd::strcmp(lhs.str, rhs.str);
    }
    friend bool operator<(string_view lhs, string_view rhs) {
        return lhs.str < rhs.str;
    }
    friend bool operator>(string_view lhs, string_view rhs) {
        return lhs.str > rhs.str;
    }

    const char* str = nullptr;
    size_t len = 0;

    inline static const size_t npos = (size_t)-1;
};

inline string& string::operator=(string_view str) {
    resize(pstd::size(str));
    pstd::copy(pstd::begin(str), pstd::end(str), begin());
    return *this;
}

inline string& string::operator+=(class string_view rhs) {
    size_t oldlen = len;

    resize(len + pstd::size(rhs));
    pstd::copy(pstd::begin(rhs), pstd::end(rhs), begin() + oldlen);
    return *this;
}
inline string& string::operator+=(const char* rhs) {
    return (*this) += string_view(rhs);
}
inline string operator+(string lhs, string_view rhs) {
    return lhs += rhs;
}

// to_string
template <typename T>
inline string to_string(T val, enable_if_t<is_integral_v<T>>* = 0);
template <typename T>
inline string to_string(T val, enable_if_t<is_floating_point_v<T>>* = 0);
template <typename T>
inline string to_string(T* val, enable_if_t<is_pointerish_v<T>>* = 0);
template <typename T>
inline string to_string(const T& val, decltype(pstd::begin(pstd::declval<T>()))* = 0);
template <typename T>
inline string to_string(const T& val, enable_if_t<has_archive_method_v<T>>* = 0, void* = 0);
template <typename... Ts, typename = enable_if_t<(sizeof...(Ts) > 1)>>
inline string to_string(const Ts&... vals);

inline string to_string(const string& val) {
    return val;
}
inline string to_string(const char* val) {
    return (string)val;
}
inline string to_string(string_view val) {
    return (string)val;
}
inline string to_string(bool val) {
    return val ? "true" : "false";
}
inline string to_string(char val) {
    return string(1, val);
}

template <typename T>
inline string to_string(T val, enable_if_t<is_integral_v<T>>*) {
    constexpr int MaxLen = 16;
    char str[MaxLen] = {};
    int i = MaxLen;

    bool negative = val < 0;
    val = pstd::abs(val);
    do {
        str[--i] = '0' + val % 10;
        val /= 10;
    } while (val && i > 1);

    if (negative)
        str[--i] = '-';

    return string(str + i, MaxLen - i);
}

template <typename T>
inline string to_string(T val, enable_if_t<is_floating_point_v<T>>*) {
    string str = pstd::to_string((corresponding_int_t<T>)val) + ".";
    val = pstd::absfract(val);

    for (int i = 0; i < 8; ++i) {
        val *= 10;
        str.push_back('0' + (char)val);
        val = pstd::absfract(val);
    }

    return str;
}

template <typename T>
inline string to_string(T* val, enable_if_t<is_pointerish_v<T>>*) {
    return "*" + pstd::to_string(*val);
}

template <typename T>
inline string to_string(const T& val, decltype(pstd::begin(pstd::declval<T>()))*) {
    string str = "[";
    for (auto it = pstd::begin(val);;) {
        str += pstd::to_string(*it);
        ++it;
        if (it != pstd::end(val))
            str += ", ";
        else
            break;
    }

    str += "]";
    return str;
}

template <typename T>
inline string to_string(const T& val, enable_if_t<has_archive_method_v<T>>*, void*) {
    string str = "{";

    pstd::apply_fields(val,
                       [&](const auto&... xs) { str += ((pstd::to_string(xs) + ", ") + ...); });

    if (pstd::size(str) > 3) {
        str.pop_back();
        str.pop_back();
    }

    return str + "}";
}

template <typename... Ts, typename>
inline string to_string(const Ts&... vals) {
    return (pstd::to_string(vals) + ...);
}

inline int stoi(string_view str) {
    int number = 0;
    bool is_neg = false;
    for (size_t j = 0; j < pstd::size(str); j++) {
        if (j == 0 && str[j] == '-')
            is_neg = true;
        else
            number = number * 10 + str[j] - '0';
    }
    return is_neg ? -number : number;
}

inline float stof(string_view str) {
    float number = 0.0f;
    bool is_neg = false;
    bool reached_dicimal_point = false;
    float scale = 0.1f;
    for (size_t j = 0; j < pstd::size(str); j++) {
        if (j == 0 && str[j] == '-')
            is_neg = true;
        else if (!reached_dicimal_point && str[j] == '.')
            reached_dicimal_point = true;
        else if (!reached_dicimal_point)
            number = number * 10 + str[j] - '0';
        else {
            number += (str[j] - '0') * scale;
            scale *= 0.1f;
        }
    }
    return is_neg ? -number : number;
}

}  // namespace pstd

#endif  // PINE_STD_STRING_H