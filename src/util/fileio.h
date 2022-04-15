#ifndef PINE_UTIL_FILIO_H
#define PINE_UTIL_FILIO_H

#include <core/geometry.h>
#include <util/archive.h>

#include <fstream>
#include <pstd/string.h>
#include <pstd/vector.h>
#include <pstd/memory.h>

namespace pine {

struct ScopedFile {
    ScopedFile(pstd::string filename, std::ios::openmode mode);

    void Write(const void* data, size_t size);
    void Read(void* data, size_t size);
    size_t Size() const;
    bool Success() const {
        return file.is_open() && file.good();
    }
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

    mutable std::fstream file;
    mutable size_t size = (size_t)-1;
};

bool IsFileExist(pstd::string filename);
pstd::string GetFileDirectory(pstd::string filename);
pstd::string GetFileExtension(pstd::string filename);
pstd::string RemoveFileExtension(pstd::string filename);
pstd::string ChangeFileExtension(pstd::string filename, pstd::string ext);
pstd::string AppendFileName(pstd::string filename, pstd::string content);
pstd::string ReadStringFile(pstd::string filename);
void WriteBinaryData(pstd::string filename, const void* ptr, size_t size);
pstd::vector<char> ReadBinaryData(pstd::string filename);

void SaveImage(pstd::string filename, vec2i size, int nchannel, float* data);
void SaveImage(pstd::string filename, vec2i size, int nchannel, uint8_t* data);
vec3u8* ReadLDRImage(pstd::string filename, vec2i& size);

TriangleMesh LoadObj(pstd::string filename);
pstd::pair<pstd::vector<float>, vec3i> LoadVolume(pstd::string filename);
void CompressVolume(pstd::string filename, const pstd::vector<float>& densityf, vec3i size);
pstd::pair<pstd::vector<float>, vec3i> LoadCompressedVolume(pstd::string filename);

Parameters LoadScene(pstd::string filename, Scene* scene);

template <typename... Ts>
void Serialize(pstd::string filename, const Ts&... object) {
    auto data = Archive(object...);
    WriteBinaryData(filename, data.data(), data.size() * sizeof(data[0]));
}
template <typename... Ts>
auto Deserialize(pstd::string filename) {
    return Unarchive<Ts...>(ReadBinaryData(filename));
}

}  // namespace pine

#endif  // PINE_UTIL_FILIO_H