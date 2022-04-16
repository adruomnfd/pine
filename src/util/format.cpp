#include <util/format.h>

#include <iostream>

namespace pine {

pstd::string ParseFormat(const char*& format, Format& fmt) {
    pstd::string formatted;
    size_t size = 0;

    int before = 0;
    while (true) {
        if (format[before] == '\0')
            break;
        if (format[before] == '&') {
            if (format[before + 1] == '&') {
                size++;
                before += 2;
                continue;
            } else {
                break;
            }
        }
        size++;
        before++;
    }

    formatted.resize(size);
    pstd::fill(formatted, ' ');
    size = 0;
    before = 0;

    while (true) {
        if (format[before] == '\0')
            break;
        if (format[before] == '&') {
            if (format[before + 1] == '&') {
                formatted[size++] = format[before];
                before += 2;
                continue;
            } else {
                break;
            }
        }
        formatted[size++] = format[before];
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
    if (pstd::isnumber(format[offset])) {
        fmt.minWidth = format[offset] - '0';
        offset++;

        if (pstd::isnumber(format[offset])) {
            fmt.minWidth = fmt.minWidth * 10 + format[offset] - '0';
            offset++;
        }
    }
    if (format[offset] == '.') {
        offset++;
    }
    if (pstd::isnumber(format[offset])) {
        fmt.precision = format[offset] - '0';
        offset++;

        if (pstd::isnumber(format[offset])) {
            fmt.precision = fmt.precision * 10 + format[offset] - '0';
            offset++;
        }
    }

    return formatted;
}

pstd::string FormatCharArray(pstd::string_view str, int minWidth, bool leftAlign) {
    pstd::string formatted;

    size_t size = 0;
    for (size_t i = 0; i < pstd::size(str);) {
        if (str[i] == '&') {
            if (str[i + 1] == '&') {
                size++;
                i += 2;
                continue;
            }
        }
        size++;
        i++;
    }

    int offset = leftAlign ? 0 : pstd::max(minWidth, (int)size) - size;
    formatted.resize(pstd::max(minWidth, (int)size));
    pstd::fill(formatted, ' ');

    int numChars = 0;
    for (size_t i = 0; i < pstd::size(str);) {
        if (str[i] == '&') {
            if (str[i + 1] == '&') {
                formatted[offset + numChars++] = str[i];
                i += 2;
                continue;
            }
        }
        formatted[offset + numChars++] = str[i];
        i++;
    }

    return formatted;
}

}  // namespace pine