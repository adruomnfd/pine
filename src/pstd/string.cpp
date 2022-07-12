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

int stoi(string_view str) {
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

float stof(string_view str) {
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

// TODO
void stois(string_view str, int* ptr, int N) {
    int dim = 0;
    int start = -1;
    for (int i = 0; i < (int)str.size(); i++) {
        if (pstd::isnumber(str[i]) || str[i] == '-') {
            if (start == -1)
                start = i;
        } else if (start != -1) {
            ptr[dim++] = pstd::stoi(string_view(str.data() + start, i - start));
            start = -1;
            if (dim == N)
                return;
        }
    }

    if (start != -1)
        ptr[dim++] = pstd::stoi(pstd::trim(str, start, (int)str.size() - start));

    if (dim == 0) {
        ptr[0] = 0;
        dim++;
    }
    for (int i = dim; i < N; i++)
        ptr[i] = ptr[dim - 1];
}

void stofs(string_view str, float* ptr, int N) {
    int dim = 0;
    int start = -1;
    for (int i = 0; i < (int)str.size(); i++) {
        if (pstd::isnumber(str[i]) || str[i] == '-' || str[i] == '.') {
            if (start == -1)
                start = i;
        } else if (start != -1) {
            ptr[dim++] = pstd::stof(string_view(str.data() + start, i - start));
            start = -1;
            if (dim == N)
                return;
        }
    }

    if (start != -1)
        ptr[dim++] = pstd::stof(pstd::trim(str, start, (int)str.size() - start));

    if (dim == 0) {
        ptr[0] = 0;
        dim++;
    }
    for (int i = dim; i < N; i++)
        ptr[i] = ptr[dim - 1];
}

}  // namespace pstd