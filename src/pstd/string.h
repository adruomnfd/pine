#ifndef PINE_STD_STRING_H
#define PINE_STD_STRING_H

#include <pstd/math.h>
#include <pstd/vector.h>
#include <pstd/archive.h>
#include <pstd/type_traits.h>

namespace pstd {

inline size_t strlen(const char* str) {
    size_t len = 0;
    while (*(str++) != '\0')
        ++len;
    return len;
}

inline bool issame(const char* lhs, const char* rhs) {
    if (!lhs || !rhs)
        return !lhs && !rhs;

    for (size_t i = 0;; ++i) {
        if (lhs[i] != rhs[i])
            return false;
        if (lhs[i] == '\0' || rhs[i] == '\0')
            return lhs[i] == '\0' && rhs[i] == '\0';
    }
}

template <typename T>
struct string_alloc {
    T* alloc(size_t size) const {
        return new T[size + 1]();
    }
    void free(T* ptr) const {
        delete[] ptr;
    }
};

class string : public vector_base<char, string_alloc<char>> {
  public:
    using base = vector_base<char, string_alloc<char>>;
    using base::base;

    string(const char* cstr) : base(strlen(cstr)) {
        copy(cstr, cstr + len, begin());
    }

    string(const char* cstr, size_t len) : base(len) {
        copy(cstr, cstr + len, begin());
    }

    string& operator=(const char* str) {
        resize(strlen(str));
        copy(str, str + size(), begin());
        return *this;
    }
    string& operator=(class string_view str);

    string substr(size_t pos) const {
        return string(data() + pos, len - pos);
    }
    string substr(size_t pos, size_t len) const {
        return string(data() + pos, len);
    }

    // TO-DO: DRY
    size_t find_first_of(char c) const {
        size_t i = 0;
        while (i < size()) {
            if (ptr[i] == c)
                break;
            i++;
        }
        if (i == size())
            return npos;

        return i;
    }
    size_t find_last_of(char c) const {
        size_t i = 0, last = npos;
        while (i < size()) {
            if (ptr[i] == c)
                last = i;
            i++;
        }

        return last;
    }

    string& operator+=(const string& rhs) {
        size_t oldlen = len;

        resize(len + rhs.len);
        copy(rhs.begin(), rhs.end(), begin() + oldlen);
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
        return issame(lhs.data(), rhs.data());
    }
    friend bool operator!=(const string& lhs, const string& rhs) {
        return !issame(lhs.data(), rhs.data());
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
    string_view(const char* str) : str(str), len(strlen(str)) {
    }
    string_view(const char* str, size_t len) : str(str), len(len) {
    }
    string_view(const string& str) : str(str.c_str()), len(str.size()) {
    }

    string_view substr(size_t pos) const {
        return string_view(str + pos, len - pos);
    }
    string_view substr(size_t pos, size_t len) const {
        // TODO zeroize ?
        return string_view(str + pos, len);
    }
    // TO-DO: DRY
    size_t find_first_of(char c) const {
        size_t i = 0;
        while (i < size()) {
            if (str[i] == c)
                break;
            i++;
        }

        if (i == size())
            i = npos;

        return i;
    }
    size_t find_last_of(char c) const {
        size_t i = 0, last = npos;
        while (i < size()) {
            if (str[i] == c)
                last = i;
            i++;
        }

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
        return issame(lhs.str, rhs.str);
    }
    friend bool operator!=(string_view lhs, string_view rhs) {
        return !issame(lhs.str, rhs.str);
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
    resize(str.size());
    copy(str.begin(), str.end(), begin());
    return *this;
}

inline string& string::operator+=(class string_view rhs) {
    size_t oldlen = len;

    resize(len + rhs.size());
    copy(rhs.begin(), rhs.end(), begin() + oldlen);
    return *this;
}
inline string& string::operator+=(const char* rhs) {
    return (*this) += string_view(rhs);
}
inline string operator+(string lhs, string_view rhs) {
    return lhs += rhs;
}

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
inline string to_string(T val, enable_if_t<is_integral_v<T>>* = 0) {
    constexpr int MaxLen = 16;
    char str[MaxLen] = {};
    int i = MaxLen;

    bool negative = val < 0;
    val = abs(val);
    do {
        str[--i] = '0' + val % 10;
        val /= 10;
    } while (val && i > 1);

    if (negative)
        str[--i] = '-';

    return string(str + i, MaxLen - i);
}

template <typename T>
inline string to_string(T val, enable_if_t<is_floating_point_v<T>>* = 0) {
    string str = pstd::to_string((corresponding_int_t<T>)val) + ".";
    val = absfract(val);

    for (int i = 0; i < 8; ++i) {
        val *= 10;
        str.push_back('0' + (char)val);
        val = absfract(val);
    }

    return str;
}

template <typename T>
inline string to_string(const T& val, decltype(declval<T>().begin())* = 0) {
    string str = "[";
    for (auto it = val.begin();;) {
        str += to_string(*it);
        ++it;
        if (it != val.end())
            str += ", ";
        else
            break;
    }

    str += "]";
    return str;
}

template <typename T>
inline string to_string(const T& val, enable_if_t<has_archive_method_v<T>>* = 0) {
    string str = "{";

    apply_fields(val, [&](const auto&... xs) { str += ((to_string(xs) + ", ") + ...); });

    if (str.size() > 3) {
        str.pop_back();
        str.pop_back();
    }

    return str + "}";
}

template <typename... Ts>
inline string to_string(const Ts&... vals) {
    return (to_string(vals) + ...);
}

inline int stoi(string_view str) {
    int number = 0;
    bool is_neg = false;
    for (size_t j = 0; j < str.size(); j++) {
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
    for (size_t j = 0; j < str.size(); j++) {
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