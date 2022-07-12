#include <pstd/iostream.h>

#include <stdio.h>

namespace pstd {

void ostream::write(const char* str, size_t size) {
    fwrite(str, size, 1, stdout);
    fflush(stdout);
}

}  // namespace pstd