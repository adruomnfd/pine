#ifndef PINE_STD_FSTREAM_H
#define PINE_STD_FSTREAM_H

#include <pstd/string.h>

#include <stdio.h>

namespace pstd {

namespace ios {

enum mode { in = 1 << 0, out = 1 << 1, binary = 1 << 2 };

};

class fstream {
  public:
    fstream(pstd::string_view filename, ios::mode mode) {
        open(filename, mode);
    }
    ~fstream() {
        close();
    }

    fstream(const fstream&) = delete;
    fstream(fstream&&) = delete;
    fstream& operator=(const fstream&) = delete;
    fstream& operator=(fstream&&) = delete;

    void open(pstd::string_view filename, ios::mode mode) {
        close();
        if (mode & ios::in)
            file = fopen(filename, mode & ios::binary ? "rb" : "r");
        else if (mode & ios::out)
            file = fopen(filename, mode & ios::binary ? "wb" : "w");
        else
            file = fopen(filename, mode & ios::binary ? "w+b" : "w+");
    }

    void close() {
        if (file) {
            fclose(file);
            file = nullptr;
        }
    }

    void write(const void* data, size_t size) {
        fwrite(data, size, 1, file);
    }
    void read(void* data, size_t size) const {
        fread(data, size, 1, file);
    }

  private:
    FILE* file = nullptr;
};

}  // namespace pstd

#endif  // PINE_STD_FSTREAM_H