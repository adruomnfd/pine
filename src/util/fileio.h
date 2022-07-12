#ifndef PINE_UTIL_FILIO_H
#define PINE_UTIL_FILIO_H

#include <core/geometry.h>
#include <util/archive.h>

#include <pstd/fstream.h>
#include <pstd/string.h>
#include <pstd/vector.h>

namespace pine {

struct ScopedFile {
    ScopedFile(pstd::string_view filename, pstd::ios::openmode mode);

    template <typename T>
    T Read() {
        T val;
        Read(&val, sizeof(T));
        return val;
    }
    template <typename T>
    void Write(const T& val) {
        Write(&val, sizeof(T));
    }

    void Write(const void* data, size_t size);
    void Read(void* data, size_t size);

    size_t Size() const {
        return file.size();
    }
    bool Success() const {
        return file.is_open();
    }

    mutable pstd::fstream file;
    mutable size_t size = -1;
};

bool IsFileExist(pstd::string_view filename);
pstd::string GetFileDirectory(pstd::string_view filename);
pstd::string GetFileExtension(pstd::string_view filename);
pstd::string RemoveFileExtension(pstd::string_view filename);
pstd::string ChangeFileExtension(pstd::string_view filename, pstd::string ext);
pstd::string AppendFileName(pstd::string_view filename, pstd::string content);

pstd::string ReadStringFile(pstd::string_view filename);
void WriteBinaryData(pstd::string_view filename, const void* ptr, size_t size);
pstd::vector<char> ReadBinaryData(pstd::string_view filename);

void SaveImage(pstd::string_view filename, vec2i size, int nchannel, const float* data);
void SaveImage(pstd::string_view filename, vec2i size, int nchannel, const uint8_t* data);
vec3u8* ReadLDRImage(pstd::string_view filename, vec2i& size);

pstd::pair<pstd::vector<float>, vec3i> LoadVolume(pstd::string_view filename);
void CompressVolume(pstd::string_view filename, const pstd::vector<float>& densityf, vec3i size);
pstd::pair<pstd::vector<float>, vec3i> LoadCompressedVolume(pstd::string_view filename);

Parameters LoadScene(pstd::string_view filename, Scene* scene);

template <typename... Ts>
void Serialize(pstd::string_view filename, const Ts&... object) {
    auto data = Archive(object...);
    WriteBinaryData(filename, data.data(), data.size() * sizeof(data[0]));
}
template <typename... Ts>
auto Deserialize(pstd::string_view filename) {
    return Unarchive<Ts...>(ReadBinaryData(filename));
}

}  // namespace pine

#endif  // PINE_UTIL_FILIO_H