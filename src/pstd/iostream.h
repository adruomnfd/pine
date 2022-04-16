#ifndef PINE_STD_IOSTREAM_H
#define PINE_STD_IOSTREAM_H

#include <pstd/string.h>

namespace pstd {

class ostream {
  public:
    template <typename T>
    friend ostream& operator<<(ostream& os, T&& val) {
        string str = pstd::to_string(pstd::forward<T>(val));
        write(str.c_str(), pstd::size(str) * sizeof(str[0]));
        return os;
    }

  private:
    static void write(const char* str, size_t size);
};

inline ostream cout;
inline constexpr char endl = '\n';

}  // namespace pstd

#endif  // PINE_STD_IOSTREAM_H