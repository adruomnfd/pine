#ifndef PINE_UTIL_FILIO_H
#define PINE_UTIL_FILIO_H

#include <core/geometry.h>
#include <util/archive.h>

#include <fstream>
#include <string>
#include <vector>
#include <memory>

namespace pine {

struct ScopedFile {
    ScopedFile(std::string filename, std::ios::openmode mode);

    void Write(const void* data, size_t size);
    void Read(void* data, size_t size);
    size_t Size() const;
    bool Success() const {
        return file.is_open() && file.good();
    }

    mutable std::fstream file;
    mutable size_t size = (size_t)-1;
};

bool IsFileExist(std::string filename);
std::string GetFileDirectory(std::string filename);
std::string GetFileExtension(std::string filename);
std::string RemoveFileExtension(std::string filename);
std::string ChangeFileExtension(std::string filename, std::string ext);
std::string AppendFileName(std::string filename, std::string content);
std::string ReadStringFile(std::string filename);
void WriteBinaryData(std::string filename, const char* ptr, size_t size);
std::vector<char> ReadBinaryData(std::string filename);

void SaveImage(std::string filename, vec2i size, int nchannel, float* data);
void SaveImage(std::string filename, vec2i size, int nchannel, uint8_t* data);
vec3u8* ReadLDRImage(std::string filename, vec2i& size);

std::vector<TriangleMesh> LoadObj(std::string filename, Scene* scene);
Parameters LoadScene(std::string filename, Scene* scene);

template <typename T>
void Serialize(std::string filename, const T& object) {
    Serializer serializer;
    auto data = serializer.Archive(object);
    WriteBinaryData(filename, data.data(), data.size() * sizeof(data[0]));
}
template <typename T>
T Deserialize(std::string filename) {
    Deserializer deserializer(ReadBinaryData(filename));
    T object;
    deserializer.Archive(object);
    return object;
}

}  // namespace pine

#endif  // PINE_UTIL_FILIO_H