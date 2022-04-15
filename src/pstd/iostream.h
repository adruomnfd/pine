#ifndef PINE_STD_IOSTREAM_H
#define PINE_STD_IOSTREAM_H

#include <stdio.h>

#include <pstd/string.h>

namespace pstd {

class ostream {
  public:
    template <typename T>
    friend ostream& operator<<(ostream& os, T&& val) {
        string str = pstd::to_string(pstd::forward<T>(val));
        fwrite(str.c_str(), pstd::size(str) * sizeof(str[0]), 1, stdout);
        return os;
    }
};

inline ostream cout;
inline constexpr char endl = '\n';

}  // namespace pstd

#endif  // PINE_STD_IOSTREAM_H