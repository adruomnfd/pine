#include <util/string.h>

namespace pine {

int StrToInt(const char* str, int len) {
    int number = 0;
    bool isNeg = false;
    for (int j = 0; j < len; j++) {
        if (j == 0 && str[j] == '-')
            isNeg = true;
        else
            number = number * 10 + str[j] - '0';
    }
    return isNeg ? -number : number;
}

float StrToFloat(const char* str, int len) {
    float number = 0.0f;
    bool isNeg = false;
    bool reachDicimalPoint = false;
    float scale = 0.1f;
    for (int j = 0; j < len; j++) {
        if (j == 0 && str[j] == '-')
            isNeg = true;
        else if (!reachDicimalPoint && str[j] == '.')
            reachDicimalPoint = true;
        else if (!reachDicimalPoint)
            number = number * 10 + str[j] - '0';
        else {
            number += (str[j] - '0') * scale;
            scale *= 0.1f;
        }
    }
    return isNeg ? -number : number;
}

void StrToInts(std::string_view str, int* ptr, int N) {
    int dim = 0;
    int start = -1;
    for (int i = 0; i < (int)str.size(); i++) {
        if (IsNumber(str[i]) || str[i] == '-') {
            if (start == -1)
                start = i;
        } else if (start != -1) {
            ptr[dim++] = StrToInt(str.data() + start, i - start);
            start = -1;
            if (dim == N)
                return;
        }
    }

    if (start != -1)
        ptr[dim++] = atoi(str.substr(start, (int)str.size() - start).data());

    if (dim == 0) {
        ptr[0] = 0;
        dim++;
    }
    for (int i = dim; i < N; i++)
        ptr[i] = ptr[dim - 1];
}

void StrToFloats(std::string_view str, float* ptr, int N) {
    int dim = 0;
    int start = -1;
    for (int i = 0; i < (int)str.size(); i++) {
        if (IsNumber(str[i]) || str[i] == '-' || str[i] == '.') {
            if (start == -1)
                start = i;
        } else if (start != -1) {
            ptr[dim++] = StrToFloat(str.data() + start, i - start);
            start = -1;
            if (dim == N)
                return;
        }
    }

    if (start != -1)
        ptr[dim++] = atof(str.substr(start, (int)str.size() - start).data());

    if (dim == 0) {
        ptr[0] = 0;
        dim++;
    }
    for (int i = dim; i < N; i++)
        ptr[i] = ptr[dim - 1];
}

Fstring& Fstring::operator+=(const Fstring& rhs) {
    int newSize = size_ + rhs.size_;
    char* newPtr = new char[newSize + 1];

    for (int i = 0; i < size_; i++)
        newPtr[i] = ptr_[i];
    for (int i = size_; i < newSize; i++)
        newPtr[i] = rhs.ptr_[i - size_];
    newPtr[newSize] = '\0';

    delete[] ptr_;
    size_ = newSize;
    ptr_ = newPtr;
    return *this;
}

Fstring Fstring::FormattingCharArray(const char* str, int minWidth, bool leftAlign) {
    Fstring formatted;
    for (int i = 0;;) {
        if (str[i] == '\0')
            break;
        if (str[i] == '&') {
            if (str[i + 1] == '&') {
                formatted.size_++;
                i += 2;
                continue;
            }
        }
        formatted.size_++;
        i++;
    }

    int offset = leftAlign ? 0 : std::max(minWidth, formatted.size_) - formatted.size_;
    formatted.size_ = std::max(minWidth, formatted.size_);
    formatted.ptr_ = new char[formatted.size_ + 1];
    for (int i = 0; i < formatted.size_; i++)
        formatted.ptr_[i] = ' ';

    int numChars = 0;
    for (int i = 0;;) {
        if (str[i] == '\0')
            break;
        if (str[i] == '&') {
            if (str[i + 1] == '&') {
                formatted.ptr_[offset + numChars++] = str[i];
                i += 2;
                continue;
            }
        }
        formatted.ptr_[offset + numChars++] = str[i];
        i++;
    }

    formatted.ptr_[formatted.size_] = '\0';
    return formatted;
}

}  // namespace pine