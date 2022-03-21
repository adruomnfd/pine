#include <core/scene.h>
#include <util/fileio.h>
#include <util/parser.h>
#include <util/archive.h>
#include <util/profiler.h>
#include <util/log.h>
#include <util/misc.h>

#include <algorithm>
#include <sstream>
#include <vector>

#include <ext/stb_image_write.h>
#include <ext/stb_image.h>

namespace pine {

std::string sceneDirectory = "";

ScopedFile::ScopedFile(std::string filename, std::ios::openmode mode) {
    filename = sceneDirectory + filename;
    std::replace(filename.begin(), filename.end(), '\\', '/');
    CHECK(filename != "");
    file.open(filename, mode);
    if (file.is_open() == false)
        LOG_WARNING("[ScopedFile]Can not open file \"&\"", filename.c_str());

    if (file.bad())
        LOG_WARNING("[ScopedFile]Bad file \"&\"", filename.c_str());
}
void ScopedFile::Write(const void *data, size_t size) {
    if (file.is_open() && file.good())
        file.write((char *)data, size);
}
void ScopedFile::Read(void *data, size_t size) {
    if (file.is_open() && file.good())
        file.read((char *)data, size);
}
size_t ScopedFile::Size() const {
    if (size != (size_t)-1)
        return size;

    size_t pos = file.tellg();
    file.seekg(0, file.end);
    size = file.tellg();
    file.seekg(pos);
    return size;
}

bool IsFileExist(std::string filename) {
    std::replace(filename.begin(), filename.end(), '\\', '/');
    std::ifstream file(filename);
    return file.good();
}
std::string GetFileDirectory(std::string filename) {
    std::replace(filename.begin(), filename.end(), '\\', '/');

    size_t forwardslash = filename.find_last_of('/');
    if (forwardslash != filename.npos)
        filename = filename.substr(0, forwardslash);

    return filename + '/';
}
std::string GetFileExtension(std::string filename) {
    size_t p = filename.find_last_of('.');
    if (p == filename.npos)
        return "";

    return filename.substr(p + 1, filename.size() - p - 1);
}
std::string RemoveFileExtension(std::string filename) {
    size_t p = filename.find_last_of('.');
    if (p == filename.npos)
        return "";

    return filename.substr(0, p);
}
std::string ChangeFileExtension(std::string filename, std::string ext) {
    return RemoveFileExtension(filename) + "." + ext;
}
std::string AppendFileName(std::string filename, std::string content) {
    return RemoveFileExtension(filename) + content + "." + GetFileExtension(filename);
}

std::string ReadStringFile(std::string filename) {
    ScopedFile file(filename, std::ios::in | std::ios::binary);
    size_t size = file.Size();
    std::string str;
    str.resize(size);
    file.Read(str.data(), size);
    return str;
}
void WriteBinaryData(std::string filename, const char *ptr, size_t size) {
    ScopedFile file(filename, std::ios::binary | std::ios::out);
    file.Write(ptr, size);
}
std::vector<char> ReadBinaryData(std::string filename) {
    ScopedFile file(filename, std::ios::binary | std::ios::in);
    std::vector<char> data(file.Size());
    file.Read(&data[0], file.Size());
    return data;
}

void SaveBMPImage(std::string filename, vec2i size, int nchannel, uint8_t *data) {
    LOG_VERBOSE("[FileIO]Saving image \"&\"", filename);
    ScopedFile file(filename, std::ios::out);

    std::vector<vec3u8> colors(size.x * size.y);
    for (int x = 0; x < size.x; x++)
        for (int y = 0; y < size.y; y++) {
            vec3u8 c;
            c.z = data[(x + y * size.x) * nchannel + 0];
            c.y = data[(x + y * size.x) * nchannel + 1];
            c.x = data[(x + y * size.x) * nchannel + 2];
            colors[x + (size.y - 1 - y) * size.x] = c;
        }

    int filesize = 54 + 3 * size.x * size.y;
    uint8_t header[] = {'B',
                        'M',
                        (uint8_t)(filesize),
                        (uint8_t)(filesize >> 8),
                        (uint8_t)(filesize >> 16),
                        (uint8_t)(filesize >> 24),
                        0,  // bfReserved1
                        0,
                        0,  // bfReserved2
                        0,
                        54,  // bfOffBits
                        0,
                        0,
                        0,
                        40,  // biSize
                        0,
                        0,
                        0,
                        (uint8_t)(size.x),
                        (uint8_t)(size.x >> 8),
                        (uint8_t)(size.x >> 16),
                        (uint8_t)(size.x >> 24),
                        (uint8_t)(size.y),
                        (uint8_t)(size.y >> 8),
                        (uint8_t)(size.y >> 16),
                        (uint8_t)(size.y >> 24),
                        1,  // biPlanes
                        0,
                        24,  // biBitCount
                        0,
                        0,  // biCompression
                        0,
                        0,
                        0,
                        0,  // biSizeImage
                        0,
                        0,
                        0,
                        0,  // biXPelsPerMeter
                        0,
                        0,
                        0,
                        0,  // biYPelsPerMeter
                        0,
                        0,
                        0,
                        0,  // biClrUsed
                        0,
                        0,
                        0,
                        0,  // biClrImportance
                        0,
                        0,
                        0};
    file.Write(header, sizeof(header));

    uint8_t padding[3] = {0, 0, 0};
    size_t paddingSize = (4 - (size.x * 3) % 4) % 4;
    if (paddingSize == 0) {
        file.Write(colors.data(), sizeof(colors[0]) * colors.size());
    } else {
        for (int y = 0; y < size.y; y++) {
            file.Write(colors.data() + y * size.x * 3, size.x * 3);
            file.Write(padding, paddingSize);
        }
    }
}
void SaveImage(std::string filename, vec2i size, int nchannel, float *data) {
    std::vector<uint8_t> pixels(size.x * size.y * nchannel);
    for (int x = 0; x < size.x; x++)
        for (int y = 0; y < size.y; y++)
            for (int c = 0; c < nchannel; c++)
                pixels[y * size.x * nchannel + x * nchannel + c] =
                    Clamp(data[y * size.x * nchannel + x * nchannel + c] * 256.0f, 0.0f, 255.0f);

    SWITCH(GetFileExtension(filename)) {
        CASE("bmp") SaveBMPImage(filename, size, nchannel, (uint8_t *)pixels.data());
        CASE("png")
        stbi_write_png(filename.c_str(), size.x, size.y, nchannel, pixels.data(),
                       size.x * nchannel);
        DEFAULT
        LOG_WARNING("& has unsupported image file extension", filename);
    }
}
void SaveImage(std::string filename, vec2i size, int nchannel, uint8_t *data) {
    SWITCH(GetFileExtension(filename)) {
        CASE("bmp") SaveBMPImage(filename, size, nchannel, data);
        CASE("png")
        stbi_write_png(filename.c_str(), size.x, size.y, nchannel, data, size.x * nchannel);
        DEFAULT
        LOG_WARNING("& has unsupported image file extension", filename);
    }
}
vec3u8 *ReadLDRImage(std::string filename, vec2i &size) {
    int nchannel = 0;
    uint8_t *data =
        (uint8_t *)stbi_load((sceneDirectory + filename).c_str(), &size.x, &size.y, &nchannel, 3);
    if (!data)
        LOG_WARNING("[FileIO]Failed to load \"&\"", filename);
    return (vec3u8 *)data;
}

TriangleMesh LoadObj(std::string filename) {
    Profiler _("LoadObj");
    LOG_VERBOSE("[FileIO]Loading \"&\"", filename);
    Timer timer;

    TriangleMesh mesh;
    std::string raw = ReadStringFile(filename);
    std::string_view str = raw;
    LOG_VERBOSE("[FileIO]Reading .obj into & MB string in & ms", str.size() / 1000000.0,
                timer.ElapsedMs());

    std::string face;
    face.reserve(64);

    while (true) {
        size_t pos = str.find_first_of('\n');
        if (pos == str.npos)
            break;
        std::string_view line = str.substr(0, pos);
        if (line[0] == 'v' && line[1] != 't' && line[1] != 'n') {
            vec3 v;
            StrToFloats(line.substr(2), &v[0], 3);
            mesh.vertices.push_back(v);

        } else if (line[0] == 'f') {
            line = line.substr(2);
            size_t forwardSlashCount = std::count(line.begin(), line.end(), '/');
            if (forwardSlashCount == 0) {
                face = line;
            } else if (forwardSlashCount == 6) {
                face += line.substr(0, line.find_first_of('/'));
                line = line.substr(line.find_first_of(' ') + 1);
                face += ' ';
                face += line.substr(0, line.find_first_of('/'));
                line = line.substr(line.find_first_of(' ') + 1);
                face += ' ';
                face += line.substr(0, line.find_first_of('/'));
            } else {
                face += line.substr(0, line.find_first_of('/'));
                line = line.substr(line.find_first_of(' ') + 1);
                face += ' ';
                face += line.substr(0, line.find_first_of('/'));
                line = line.substr(line.find_first_of(' ') + 1);
                face += ' ';
                face += line.substr(0, line.find_first_of('/'));
                line = line.substr(line.find_first_of(' ') + 1);
                face += ' ';
                face += line.substr(0, line.find_first_of('/'));
            }

            size_t spaceCount = std::count(face.begin(), face.end(), ' ');
            if (spaceCount == 2) {
                vec3i f;
                StrToInts(face, &f[0], 3);
                for (int i = 0; i < 3; i++) {
                    if (f[i] < 0)
                        f[i] = mesh.vertices.size() + f[i];
                    else
                        f[i] -= 1;
                }
                mesh.indices.push_back(f.x);
                mesh.indices.push_back(f.y);
                mesh.indices.push_back(f.z);
            } else if (spaceCount == 3) {
                vec4i f;
                StrToInts(face.c_str(), &f[0], 4);
                for (int i = 0; i < 4; i++) {
                    if (f[i] < 0)
                        f[i] = mesh.vertices.size() + f[i];
                    else
                        f[i] -= 1;
                }
                mesh.indices.push_back(f[0]);
                mesh.indices.push_back(f[1]);
                mesh.indices.push_back(f[2]);
                mesh.indices.push_back(f[0]);
                mesh.indices.push_back(f[2]);
                mesh.indices.push_back(f[3]);
            }
        }
        face.clear();
        str = str.substr(pos + 1);
    }
    LOG("[FileIO]Loaded mesh has & M triangles", mesh.indices.size() / 3 / 1000000.0);

    LOG_VERBOSE("[FileIO]Obj loaded in & ms", timer.Reset());
    return mesh;
}

Parameters LoadScene(std::string filename, Scene *scene) {
    Parameters params = Parse(ReadStringFile(filename));
    LOG_VERBOSE("[FileIO]Creating scene from parameters");
    sceneDirectory = GetFileDirectory(filename);

    Timer timer;

    scene->camera = Camera::Create(params["Camera"], scene);

    for (auto &p : params.GetAll("Material"))
        scene->materials[p.GetString("name")] = std::make_shared<Material>(Material::Create(p));
    for (auto &p : params.GetAll("Medium"))
        scene->mediums[p.GetString("name")] = std::make_shared<Medium>(Medium::Create(p));
    for (auto &p : params.GetAll("Light"))
        scene->lights.push_back(Light::Create(p));
    for (auto &p : params.GetAll("Shape"))
        scene->shapes.push_back(Shape::Create(p, scene));
    for (auto &shape : scene->shapes)
        if (auto light = shape.GetLight())
            scene->lights.push_back(*light);
    for (const Light &light : scene->lights)
        if (light.Tag() == Light::Index<EnvironmentLight>())
            scene->envLight = light.Be<EnvironmentLight>();

    scene->integrator = Integrator::Create(params["Integrator"], scene);

    LOG_VERBOSE("[FileIO]Scene created in & ms", timer.ElapsedMs());

    return params;
}

}  // namespace pine
