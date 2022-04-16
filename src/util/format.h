#ifndef PINE_UTIL_FORMAT_H
#define PINE_UTIL_FORMAT_H

#include <core/defines.h>

#include <pstd/string.h>

namespace pine {

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

pstd::string ParseFormat(const char *&format, Format &fmt);
pstd::string FormatCharArray(pstd::string_view str, int minWidth = -1, bool leftAlign = false);
template <typename Ty>
pstd::string FormatItImpl(const Ty &value, Format fmt = {});
template <typename T>
pstd::string FormatItImpl(const char *&format, const T &first, Format fmt = {});

inline pstd::string FormatIt(pstd::string_view view) {
    return pstd::string(view);
}

template <typename T, typename... Ts>
pstd::string FormatIt(pstd::string_view view, const T &first, const Ts &...rest) {
    static_assert(!pstd::is_same_v<T, Format>, "Cannot format _Format_");
    const char *format = view.c_str();

    auto formatted = FormatItImpl(format, first);

    if constexpr (sizeof...(rest) != 0)
        return formatted + FormatIt(format, rest...);
    else
        return formatted + FormatCharArray(format);
}

template <typename T, typename... Ts>
pstd::string FormatIt(pstd::string_view view, Format fmt, const T &first, const Ts &...rest) {
    static_assert(!pstd::is_same_v<T, Format>, "Two consecutive _Format_ is not allowed");
    const char *format = view.c_str();

    auto formatted = FormatItImpl(format, first, fmt);

    if constexpr (sizeof...(rest) != 0)
        return formatted + FormatIt(format, rest...);
    else
        return formatted + FormatCharArray(format);
}

template <typename T, typename... Ts>
pstd::string FormatIt(Format fmt, pstd::string_view view, const T &first, const Ts &...rest) {
    static_assert(!pstd::is_same_v<T, Format>,
                  "local _Format_ cannot be applied when global _Format_ is specified");
    const char *format = view.c_str();

    auto formatted = FormatItImpl(format, first, fmt);

    if constexpr (sizeof...(rest) != 0)
        return formatted + FormatIt(fmt, format, rest...);
    else
        return formatted + FormatCharArray(format);
}

template <typename T, typename = void>
struct HasFormattingMethod {
    static constexpr bool value = false;
};
template <typename T>
struct HasFormattingMethod<T, pstd::voids_t<decltype(&T::Formatting)>> {
    static constexpr bool value = true;
};

template <typename Ty>
pstd::string FormatItImpl(const Ty &value, Format fmt) {
    using T = pstd::decay_t<Ty>;
    pstd::string formatted;
    size_t size = 0;

    if constexpr (HasFormattingMethod<T>::value) {
        return value.Formatting(fmt);
    } else if constexpr (pstd::is_convertible_v<T, pstd::string_view>) {
        return FormatCharArray(value, fmt.minWidth, fmt.leftAlign);
    } else if constexpr (pstd::is_same_v<T, char>) {
        formatted.resize(pstd::max(1, fmt.minWidth));
        pstd::fill(formatted, ' ');
        formatted[0] = value;
    } else if constexpr (pstd::is_arithmetic_v<T>) {
        bool negative = value < 0;

        if constexpr (pstd::is_floating_point_v<T>) {
            if (pstd::isnan(value))
                return "nan";
            else if (pstd::isinf(value))
                return negative ? "-inf" : "inf";
            else if (pstd::abs(value) > 1e+20f)
                return negative ? "-big" : "big";
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

        if constexpr (pstd::is_floating_point_v<T>) {
            if (fmt.precision < 0) {
                fmt.precision = 4;
                uint64_t digits = pstd::abs(value) * 1e+4;
                while (fmt.precision != 0 && (digits % 10) == 0) {
                    fmt.precision--;
                    digits /= 10;
                }
            }
        }

        int sizeOfIntegerPart =
            pstd::max(numDigits, fmt.minWidth) +
            ((fmt.emptySpaceIfNoSign || fmt.showPositiveSign || negative) ? 1 : 0);
        int offset = fmt.leftAlign ? (pstd::max(numDigits, fmt.minWidth) - numDigits) : 0;

        size = sizeOfIntegerPart +
               ((pstd::is_floating_point_v<T> && fmt.precision > 0) ? fmt.precision + 1 : 0);
        formatted.resize(size);

        for (size_t i = 0; i < size; i++)
            formatted[i] = fmt.padWithZero ? '0' : ' ';

        if (negative || fmt.showPositiveSign)
            formatted[sizeOfIntegerPart - numDigits - 1 - offset] = negative ? '-' : '+';

        for (int i = 0; i < numDigits; i++) {
            formatted[sizeOfIntegerPart - i - 1 - offset] = '0' + digits % 10;
            digits /= 10;
        }

        if constexpr (pstd::is_floating_point_v<T>) {
            double digits = (double)pstd::abs(value);
            if (fmt.precision > 0)
                formatted[sizeOfIntegerPart - offset] = '.';
            for (int i = 0; i < fmt.precision; i++) {
                digits *= 10;
                formatted[sizeOfIntegerPart + 1 + i - offset] = '0' + uint64_t(digits) % 10;
            }
        }

    } else if constexpr (pstd::is_pointer_v<T>) {
        formatted += "*";
        formatted += FormatItImpl(*value, fmt);
    } else if constexpr (pstd::has_archive_method_v<T>) {
        formatted += "{";
        pstd::foreach_field(value,
                            [&](auto &&field) { formatted += FormatItImpl(field, fmt) + ", "; });
        if (formatted.size() != 1) {
            formatted.pop_back();
            formatted.pop_back();
        }
        formatted += "}";
    } else if constexpr (pstd::is_iterable_v<T>) {
        formatted += "[";
        for (auto &elem : value)
            formatted += FormatItImpl(elem, fmt) + ", ";
        if (formatted.size() != 1) {
            formatted.pop_back();
            formatted.pop_back();
        }
        formatted += "]";
    } else {
        static_assert(pstd::defered_bool<T, false>::value, "Unsupported type");
    }

    return formatted;
}

template <typename T>
pstd::string FormatItImpl(const char *&format, const T &first, Format fmt) {
    pstd::string formatted = ParseFormat(format, fmt);

    formatted += FormatItImpl(first, fmt);

    ++format;

    while (*format == '<' || *format == 'o' || *format == '+' || *format == '-' || *format == '.' ||
           pstd::isnumber(*format))
        ++format;

    return formatted;
}

}  // namespace pine

#endif  // PINE_UTIL_FORMAT_H