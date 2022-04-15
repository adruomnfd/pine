#ifndef PINE_UTIL_STRING_H
#define PINE_UTIL_STRING_H

#include <core/defines.h>

#include <pstd/string.h>

namespace pine {

inline bool IsNumber(char c) {
    return c >= '0' && c <= '9';
}

inline void StrToInts(pstd::string_view str, int* ptr, int N) {
    int dim = 0;
    int start = -1;
    for (int i = 0; i < (int)str.size(); i++) {
        if (IsNumber(str[i]) || str[i] == '-') {
            if (start == -1)
                start = i;
        } else if (start != -1) {
            ptr[dim++] = pstd::stof(str.substr(start, i - start));
            start = -1;
            if (dim == N)
                return;
        }
    }

    if (start != -1)
        ptr[dim++] = pstd::stoi(str.substr(start));

    if (dim == 0) {
        ptr[0] = 0;
        dim++;
    }
    for (int i = dim; i < N; i++)
        ptr[i] = ptr[dim - 1];
}

inline void StrToFloats(pstd::string_view str, float* ptr, int N) {
    int dim = 0;
    int start = -1;
    for (int i = 0; i < (int)str.size(); i++) {
        if (IsNumber(str[i]) || str[i] == '-' || str[i] == '.') {
            if (start == -1)
                start = i;
        } else if (start != -1) {
            ptr[dim++] = pstd::stof(str.substr(start, i - start));
            start = -1;
            if (dim == N)
                return;
        }
    }

    if (start != -1)
        ptr[dim++] = pstd::stof(str.substr(start));

    if (dim == 0) {
        ptr[0] = 0;
        dim++;
    }
    for (int i = dim; i < N; i++)
        ptr[i] = ptr[dim - 1];
}

// To be implemented

}  // namespace pine

#endif  // PINE_UTIL_STRING_H