#include <pstd/string.h>

namespace pstd {

size_t strlen(const char* str) {
    if (str == nullptr)
        return 0;
    size_t len = 0;
    while (*(str++))
        ++len;
    return len;
}

int strcmp(const char* lhs, const char* rhs) {
    if (!lhs && !rhs)
        return 0;
    else if (!lhs && rhs)
        return -1;
    else if (lhs && !rhs)
        return 1;

    for (size_t i = 0;; ++i) {
        if (!lhs[i] && !rhs[i])
            return 0;
        else if (!lhs[i] && rhs[i])
            return -1;
        else if (lhs[i] && !rhs[i])
            return 1;
        else if (lhs[i] < rhs[i])
            return -1;
        else if (lhs[i] > rhs[i])
            return 1;
    }
}

int stoi(pstd::string str) {
    int number = 0;
    bool is_neg = false;
    for (size_t j = 0; j < pstd::size(str); j++) {
        if (str[j] == '.')
            break;
        if (j == 0 && str[j] == '-')
            is_neg = true;
        else
            number = number * 10 + str[j] - '0';
    }
    return is_neg ? -number : number;
}

float stof(pstd::string str) {
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