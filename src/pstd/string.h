#ifndef PINE_STD_STRING_H
#define PINE_STD_STRING_H

#include <pstd/vector.h>
#include <pstd/type_traits.h>

namespace pstd {

inline size_t strlen(const char* str) {
    size_t len = 0;
    while (*(str++) != '\0')
        ++len;
    return len;
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

    string& operator+=(const string& rhs) {
        size_t oldlen = len;

        resize(len + rhs.len);
        copy(rhs.begin(), rhs.end(), begin() + oldlen);
        return *this;
    }

    friend string operator+(string lhs, const string& rhs) {
        return lhs += rhs;
    }

    const char* c_str() const {
        return data();
    }

    friend bool operator==(const string& lhs, const string& rhs) {
        return lhs.data() == rhs.data();
    }
    friend bool operator!=(const string& lhs, const string& rhs) {
        return lhs.data() != rhs.data();
    }
    friend bool operator<(const string& lhs, const string& rhs) {
        return lhs.data() < rhs.data();
    }
    friend bool operator>(const string& lhs, const string& rhs) {
        return lhs.data() > rhs.data();
    }
};

template <typename T>
inline string to_string(T val, enable_if_t<is_convertible_v<decay_t<T>, string>>* = 0) {
    return val;
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

    do {
        str[--i] = '0' + val % 10;
        val /= 10;
    } while (val && i);

    return string(str + i, MaxLen - i);
}

template <typename T>
inline string to_string(T val, enable_if_t<is_floating_point_v<T>>* = 0) {
    string str = pstd::to_string((uint64_t)val) + ".";
    val = val - (T)(uint64_t)val;

    for (int i = 0; i < 8; ++i) {
        val *= 10;
        str += to_string(char('0' + (char)val));
        val = val - (T)(uint64_t)val;
    }

    return str;
}

template <typename T>
inline string to_string(const T& val, decltype(declval<T>().begin())* = 0,
                        decltype(declval<T>().end())* = 0,
                        enable_if_t<!is_convertible_v<decay_t<T>, string>>* = 0) {
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

}  // namespace pstd

#endif  // PINE_STD_STRING_H