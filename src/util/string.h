#ifndef PINE_UTIL_STRING_H
#define PINE_UTIL_STRING_H

#include <core/defines.h>

#include <utility>
#include <string>
#include <cmath>

namespace pine {

template <typename T>
auto ToString(T val) -> std::enable_if_t<std::is_same<T, char>::value, std::string> {
    return std::string(1, val);
}

template <typename T>
auto ToString(T val) -> std::enable_if_t<std::is_convertible<T, std::string>::value, std::string> {
    return std::string(val);
}

template <typename T>
auto ToString(T val)
    -> std::enable_if_t<std::is_same<decltype(std::to_string(val)), std::string>::value &&
                            !std::is_same<T, char>::value,
                        std::string> {
    return std::to_string(val);
}

template <typename T>
auto ToString(T val) -> decltype(val.ToString()) {
    return val.ToString();
}

template <typename... Ts>
auto ToString(Ts... vals) -> std::enable_if_t<(sizeof...(vals) > 1), std::string> {
    std::string str;
    int expand[] = {0, (str += pine::ToString(vals), 0)...};
    (void)expand;
    return str;
}

int StrToInt(const char *str, int len);
float StrToFloat(const char *str, int len);
void StrToInts(std::string_view str, int *ptr, int N);
void StrToFloats(std::string_view str, float *ptr, int N);

inline bool IsNumber(char c) {
    return c >= '0' && c <= '9';
}

struct Format {
    Format(int minWidth = -1, int precision = -1, bool leftAlign = false, bool padWithZero = false,
           bool showPositiveSign = false, bool emptySpaceIfNoSign = false)
        : minWidth(minWidth),
          precision(precision),
          leftAlign(leftAlign),
          padWithZero(padWithZero),
          showPositiveSign(showPositiveSign),
          emptySpaceIfNoSign(emptySpaceIfNoSign){};

    int minWidth;
    int precision;
    bool leftAlign;
    bool padWithZero;
    bool showPositiveSign;
    bool emptySpaceIfNoSign;
};

class FormattedString {
  public:
    FormattedString() = default;
    inline ~FormattedString();
    inline FormattedString(const FormattedString &rhs);
    inline FormattedString(FormattedString &&rhs);
    inline FormattedString &operator=(FormattedString rhs);
    inline void swap(FormattedString &rhs);
    inline friend void swap(FormattedString &lhs, FormattedString &rhs);
    FormattedString &operator+=(const FormattedString &rhs);
    inline friend FormattedString operator+(FormattedString lhs, const FormattedString &rhs);
    inline friend bool operator==(FormattedString lhs, const FormattedString &rhs);

    size_t size() const {
        return (size_t)size_;
    }
    const char *c_str() const {
        return ptr_;
    }

    FormattedString(const char *format) {
        *this = FormattingCharArray(format);
    }

    template <typename T, typename... Ts>
    FormattedString(const char *format, const T &first, const Ts &... rest) {
        static_assert(!std::is_same<T, Format>::value, "Error");

        *this = Formatting(format, first);

        if constexpr (sizeof...(rest) != 0)
            *this += FormattedString(format, rest...);
        else
            *this += FormattingCharArray(format);
    }

    template <typename T, typename... Ts>
    FormattedString(const char *format, Format fmt, const T &first, const Ts &... rest) {
        static_assert(!std::is_same<T, Format>::value, "Two consecutive _Format_ is not allowed");

        *this = Formatting(format, first, fmt);

        if constexpr (sizeof...(rest) != 0)
            *this += FormattedString(format, rest...);
        else
            *this += FormattingCharArray(format);
    }

    template <typename T, typename... Ts>
    FormattedString(Format fmt, const char *format, const T &first, const Ts &... rest) {
        static_assert(!std::is_same<T, Format>::value,
                      "local _Format_ cannot be applied when global _Format_ is specified");

        *this = Formatting(format, first, fmt);

        if constexpr (sizeof...(rest) != 0)
            *this += FormattedString(fmt, format, rest...);
        else
            *this += FormattingCharArray(format);
    }

  private:
    static FormattedString FormattingCharArray(const char *str, int minWidth = -1,
                                               bool leftAlign = false);
    template <typename Ty>
    inline static FormattedString FormattingImpl(const Ty &value, Format fmt = {});

    template <typename T>
    inline static FormattedString Formatting(const char *&format, const T &first, Format fmt = {});

    char *ptr_ = nullptr;
    int size_ = 0;
};

FormattedString::~FormattedString() {
    if (ptr_ != nullptr)
        delete[] ptr_;
}

FormattedString::FormattedString(const FormattedString &rhs) {
    size_ = rhs.size_;
    ptr_ = new char[size_];
    for (int i = 0; i < size_; i++)
        ptr_[i] = rhs.ptr_[i];
}

FormattedString::FormattedString(FormattedString &&rhs) {
    ptr_ = std::exchange(rhs.ptr_, nullptr);
    size_ = std::exchange(rhs.size_, 0);
}

FormattedString &FormattedString::operator=(FormattedString rhs) {
    swap(rhs);
    return *this;
}

void FormattedString::swap(FormattedString &rhs) {
    std::swap(ptr_, rhs.ptr_);
    std::swap(size_, rhs.size_);
}

void swap(FormattedString &lhs, FormattedString &rhs) {
    lhs.swap(rhs);
}

FormattedString operator+(FormattedString lhs, const FormattedString &rhs) {
    return lhs += rhs;
}

bool operator==(FormattedString lhs, const FormattedString &rhs) {
    if (lhs.size_ != rhs.size_)
        return false;
    for (int i = 0; i < lhs.size_; i++)
        if (lhs.ptr_[i] != rhs.ptr_[i])
            return false;

    return true;
}

template <typename T>
struct HasMemberMethodFormatting {
    template <typename U>
    static std::true_type Check(decltype(&U::Formatting));
    template <typename U>
    static std::false_type Check(...);

    static constexpr bool value = std::is_same<decltype(Check<T>(0)), std::true_type>::value;
};

template <typename Ty>
FormattedString FormattedString::FormattingImpl(const Ty &value, Format fmt) {
    using T = typename std::decay<Ty>::type;
    FormattedString formatted;

    static_assert(HasMemberMethodFormatting<T>::value || std::is_same<T, char>::value ||
                      std::is_same<T, char *>::value || std::is_same<T, const char *>::value ||
                      std::is_pointer<T>::value || std::is_null_pointer<T>::value ||
                      std::is_integral<T>::value || std::is_floating_point<T>::value ||
                      std::is_same<T, std::string>::value,
                  "Unsupported type");

    if (fmt.minWidth == 0) {
    }

    if constexpr (std::is_same<T, std::string>::value)
        formatted = FormattingCharArray(value.c_str(), fmt.minWidth, fmt.leftAlign);

    if constexpr (HasMemberMethodFormatting<T>::value) {
        formatted = value.Formatting(fmt);
    } else if constexpr (std::is_same<T, char>::value) {
        formatted.size_ = std::max(1, fmt.minWidth);
        formatted.ptr_ = new char[formatted.size_ + 1];
        formatted.ptr_[0] = value;
        formatted.ptr_[formatted.size_] = '\0';
    } else if constexpr (std::is_same<T, char *>::value || std::is_same<T, const char *>::value) {
        formatted = FormattingCharArray(value, fmt.minWidth, fmt.leftAlign);
    } else if constexpr (std::is_pointer<T>::value || std::is_null_pointer<T>::value ||
                         std::is_integral<T>::value || std::is_floating_point<T>::value) {
        bool negative;
        if constexpr (std::is_floating_point<T>::value)
            negative = value < 0.0;
        else
            negative = (int64_t)value < 0;

        if constexpr (std::is_floating_point<T>::value) {
            if (std::isnan(value)) {
                formatted.size_ = 4;
                formatted.ptr_ = new char[5];
                formatted.ptr_[0] = negative ? '-' : '+';
                formatted.ptr_[1] = 'N';
                formatted.ptr_[2] = 'a';
                formatted.ptr_[3] = 'N';
                formatted.ptr_[4] = '\0';
                return formatted;
            }
            if (std::isinf(value)) {
                formatted.size_ = 4;
                formatted.ptr_ = new char[5];
                formatted.ptr_[0] = negative ? '-' : '+';
                formatted.ptr_[1] = 'I';
                formatted.ptr_[2] = 'N';
                formatted.ptr_[3] = 'F';
                formatted.ptr_[4] = '\0';
                return formatted;
            } else if (std::abs(value) > 1e+20f) {
                formatted.size_ = 4;
                formatted.ptr_ = new char[5];
                formatted.ptr_[0] = negative ? '-' : '+';
                formatted.ptr_[1] = 'B';
                formatted.ptr_[2] = 'I';
                formatted.ptr_[3] = 'G';
                formatted.ptr_[4] = '\0';
                return formatted;
            }
        }

        auto Log10 = [](int64_t integer) {
            if (integer <= 0)
                return 0;

            int r = -1;
            while (integer > 0) {
                integer /= 10;
                r++;
            }
            return r;
        };

        int64_t digits = negative ? -(int64_t)value : (int64_t)value;
        int numDigits = Log10(digits) + 1;

        if constexpr (std::is_floating_point<T>::value) {
            if (fmt.precision < 0) {
                fmt.precision = 4;
                uint64_t digits = (double)std::abs(value) * 1e+4;
                while (fmt.precision != 0 && (digits % 10) == 0) {
                    fmt.precision--;
                    digits /= 10;
                }
            }
        }

        int sizeOfIntegerPart =
            std::max(numDigits, fmt.minWidth) +
            ((fmt.emptySpaceIfNoSign || fmt.showPositiveSign || negative) ? 1 : 0);
        int offset = fmt.leftAlign ? (std::max(numDigits, fmt.minWidth) - numDigits) : 0;

        formatted.size_ =
            sizeOfIntegerPart +
            ((std::is_floating_point<T>::value && fmt.precision > 0) ? fmt.precision + 1 : 0);
        formatted.ptr_ = new char[formatted.size_ + 1];

        for (int i = 0; i < formatted.size_; i++)
            formatted.ptr_[i] = fmt.padWithZero ? '0' : ' ';

        if (negative || fmt.showPositiveSign)
            formatted.ptr_[sizeOfIntegerPart - numDigits - 1 - offset] = negative ? '-' : '+';

        for (int i = 0; i < numDigits; i++) {
            formatted.ptr_[sizeOfIntegerPart - i - 1 - offset] = '0' + digits % 10;
            digits /= 10;
        }

        if constexpr (std::is_floating_point<T>::value) {
            double digits = (double)std::abs(value);
            if (fmt.precision > 0)
                formatted.ptr_[sizeOfIntegerPart - offset] = '.';
            for (int i = 0; i < fmt.precision; i++) {
                digits *= 10;
                formatted.ptr_[sizeOfIntegerPart + 1 + i - offset] = '0' + uint64_t(digits) % 10;
            }
        }

        formatted.ptr_[formatted.size_] = '\0';
    }

    return formatted;
}

template <typename T>
FormattedString FormattedString::Formatting(const char *&format, const T &first, Format fmt) {
    FormattedString formatted;

    int before = 0;
    while (true) {
        if (format[before] == '\0')
            break;
        if (format[before] == '&') {
            if (format[before + 1] == '&') {
                formatted.size_++;
                before += 2;
                continue;
            } else {
                break;
            }
        }
        formatted.size_++;
        before++;
    }

    formatted.ptr_ = new char[formatted.size_];
    formatted.size_ = 0;
    before = 0;

    while (true) {
        if (format[before] == '\0')
            break;
        if (format[before] == '&') {
            if (format[before + 1] == '&') {
                formatted.ptr_[formatted.size_++] = format[before];
                before += 2;
                continue;
            } else {
                break;
            }
        }
        formatted.ptr_[formatted.size_++] = format[before];
        before++;
    }

    format = format + before;
    int offset = 1;
    if (format[offset] == '<') {
        fmt.leftAlign = true;
        offset++;
    }
    if (format[offset] == 'o') {
        fmt.padWithZero = true;
        offset++;
    }
    if (format[offset] == '+') {
        fmt.showPositiveSign = true;
        offset++;
    } else if (format[offset] == '-') {
        fmt.emptySpaceIfNoSign = true;
        offset++;
    }
    if (IsNumber(format[offset])) {
        fmt.minWidth = format[offset] - '0';
        offset++;

        if (IsNumber(format[offset])) {
            fmt.minWidth = fmt.minWidth * 10 + format[offset] - '0';
            offset++;
        }
    }
    if (format[offset] == '.') {
        offset++;
    }
    if (IsNumber(format[offset])) {
        fmt.precision = format[offset] - '0';
        offset++;

        if (IsNumber(format[offset])) {
            fmt.precision = fmt.precision * 10 + format[offset] - '0';
            offset++;
        }
    }

    formatted += FormattingImpl(first, fmt);

    format = format + 1;

    while (*format == '<' || *format == 'o' || *format == '+' || *format == '-' || *format == '.' ||
           IsNumber(*format)) {
        format++;
    }

    return formatted;
}

}  // namespace pine

#endif  // PINE_UTIL_STRING_H