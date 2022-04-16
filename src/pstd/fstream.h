#ifndef PINE_STD_FSTREAM_H
#define PINE_STD_FSTREAM_H

#include <pstd/string.h>

namespace pstd {

namespace ios {

enum openmode { in = 1 << 0, out = 1 << 1, binary = 1 << 2 };

inline openmode operator|(openmode a, openmode b) {
    return openmode((int)a | (int)b);
}

}  // namespace ios

class fstream {
  public:
    fstream() = default;
    fstream(pstd::string_view filename, ios::openmode mode) {
        open(filename, mode);
    }
    ~fstream() {
        close();
    }

    fstream(const fstream&) = delete;
    fstream(fstream&&) = delete;
    fstream& operator=(const fstream&) = delete;
    fstream& operator=(fstream&&) = delete;

    void open(pstd::string_view filename, ios::openmode mode);

    void close();

    bool is_open() const;

    size_t size() const;

    void write(const void* data, size_t size);
    void read(void* data, size_t size) const;

  private:
    void* file = nullptr;
    mutable size_t size_ = -1;
};

}  // namespace pstd

#endif  // PINE_STD_FSTREAM_H