#include <pstd/fstream.h>

#include <stdio.h>

namespace pstd {

void fstream::open(pstd::string_view filename, ios::openmode mode) {
    close();
    if (mode & ios::in)
        file = fopen(filename.data(), mode & ios::binary ? "rb" : "r");
    else if (mode & ios::out)
        file = fopen(filename.data(), mode & ios::binary ? "wb" : "w");
    else
        file = fopen(filename.data(), mode & ios::binary ? "w+b" : "w+");
}

void fstream::fstream::close() {
    if (file) {
        fclose((FILE*)file);
        file = nullptr;
    }
}

bool fstream::is_open() const {
    return file != nullptr;
}

size_t fstream::size() const {
    if (!file)
        return 0;
    if (size_ == size_t(-1)) {
        size_t begin = ftell((FILE*)file);
        fseek((FILE*)file, 0, SEEK_END);
        size_t end = ftell((FILE*)file);
        fseek((FILE*)file, 0, SEEK_SET);
        size_ = end - begin;
    }

    return size_;
}

void fstream::write(const void* data, size_t size) {
    fwrite(data, size, 1, (FILE*)file);
}
void fstream::read(void* data, size_t size) const {
    size_t ret = fread(data, size, 1, (FILE*)file);
    (void)ret;
}

}  // namespace pstd