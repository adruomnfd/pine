#include <core/scene.h>
#include <util/fileio.h>
#include <util/parser.h>
#include <util/profiler.h>
#include <util/serializer.h>
#include <util/log.h>

#include <algorithm>
#include <sstream>
#include <vector>

#include <ext/stb_image_write.h>
#include <ext/stb_image.h>

namespace pine {

std::string sceneDirectory = "";

ScopedFile::ScopedFile(std::string filename, std::ios::openmode mode) {
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
    size_t pos = file.tellg();
    file.seekg(0, file.end);
    size_t size = file.tellg();
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

std::string ReadStringFile(std::string filename) {
    std::replace(filename.begin(), filename.end(), '\\', '/');
    ScopedFile file(filename, std::ios::in | std::ios::binary);
    size_t size = file.Size();
    std::string str;
    str.resize(size);
    file.Read(str.data(), size);
    return str;
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

    if (filename.size() > 3 && filename.substr(filename.size() - 3) == "bmp")
        SaveBMPImage(filename, size, nchannel, (uint8_t *)pixels.data());
    else if (filename.size() > 3 && filename.substr(filename.size() - 3) == "png")
        stbi_write_png(filename.c_str(), size.x, size.y, nchannel, pixels.data(),
                       size.x * nchannel);
    else
        LOG_WARNING("& has unsupported image file extension", filename);
}
void SaveImage(std::string filename, vec2i size, int nchannel, uint8_t *data) {
    if (filename.size() > 3 && filename.substr(filename.size() - 3) == "bmp")
        SaveBMPImage(filename, size, nchannel, data);
    else if (filename.size() > 3 && filename.substr(filename.size() - 3) == "png")
        stbi_write_png(filename.c_str(), size.x, size.y, nchannel, data, size.x * nchannel);
    else
        LOG_WARNING("& has unsupported image file extension", filename);
}
vec3u8 *ReadLDRImage(std::string filename, vec2i &size) {
    int nchannel = 0;
    vec3u8 *ptr = (vec3u8 *)stbi_load(filename.c_str(), &size.x, &size.y, &nchannel, 3);
    if (!ptr)
        LOG_WARNING("[FileIO]Failed to load \"&\"", filename);
    CHECK_EQ(nchannel, 3);
    return ptr;
}

std::vector<TriangleMesh> LoadObj(std::string filename, Scene *) {
    Profiler _("LoadObj");
    LOG_VERBOSE("[FileIO]Loading \"&\"", filename);
    Timer timer;

    std::vector<TriangleMesh> meshes;
    std::string raw = ReadStringFile(filename);
    std::string_view str = raw;
    LOG_VERBOSE("[FileIO]Reading .obj into & MB string in & ms", str.size() / 1000000.0,
                timer.ElapsedMs());

    TriangleMesh mesh;
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
    meshes.push_back(mesh);

    LOG_VERBOSE("[FileIO]Obj loaded in & ms", timer.Reset());
    return meshes;
}

Parameters LoadScene(std::string filename, Scene *scene) {
    Parameters params = Parse(ReadStringFile(filename));
    LOG_VERBOSE("[FileIO]Creating scene from parameters");
    sceneDirectory = GetFileDirectory(filename);

    Timer timer;

    scene->parameters = params["Scene"]["singleton"];
    scene->camera = Camera::Create(params["Camera"]["singleton"], scene);

    for (const auto &block : params.subset) {
        if (block.first == "Material") {
            for (const auto &subset : block.second.subset)
                scene->materials[subset.first] = Material::Create(subset.second);
        }
    }

    for (const auto &block : params.subset) {
        if (block.first == "Medium") {
            for (auto &subset : block.second.subset) {
                scene->mediums[subset.first] = Medium::Create(subset.second);
            }
        }
    }

    for (const auto &block : params.subset) {
        if (block.first == "Shape") {
            for (const auto &subset : block.second.subset) {
                if (subset.second.GetString("type") == "TriangleMesh") {
                    std::vector<TriangleMesh> meshes = LoadObj(
                        GetFileDirectory(filename) + subset.second.GetString("file"), scene);
                    for (auto &mesh : meshes) {
                        if (mesh.materialName == "")
                            mesh.material = scene->materials[subset.second.GetString("material")];
                        else
                            mesh.material = scene->materials[mesh.materialName];
                        mesh.mediumInterface.inside =
                            scene->mediums[subset.second.GetString("mediumInside")];
                        mesh.mediumInterface.outside =
                            scene->mediums[subset.second.GetString("mediumOutside")];
                        mesh.o2w = Translate(subset.second.GetVec3("translate", vec3(0.0f))) *
                                   Scale(subset.second.GetVec3("scale", vec3(1.0f)));
                        scene->meshes.push_back(mesh);
                    }
                } else {
                    scene->shapes.push_back(Shape::Create(subset.second, scene));
                }
            }
        }
    }

    for (const auto &block : params.subset) {
        if (block.first == "Light") {
            for (const auto &subset : block.second.subset)
                scene->lights.push_back(Light::Create(subset.second));
        }
    }

    scene->integrator = Integrator::Create(params["Integrator"]["singleton"], scene);

    LOG_VERBOSE("[FileIO]Scene created in & ms", timer.ElapsedMs());

    return params;
}

// void Serialize(std::string filename, const Scene *) {
//     Profiler _("Serialize");

//     LOG_VERBOSE("[FileIO]Serializing \"&\"", filename.c_str());
//     Serializer serializer;
//     scene->Archive(serializer);

//     ScopedFile file(filename, std::ios::out | std::ios::binary);
//     file.Write(serializer.data.data(), serializer.data.size());
// }

// void Deserialize(std::string filename, Scene *) {
//     Profiler _("Deserialize");

//     LOG_VERBOSE("[FileIO]Deserializing \"&\"", filename.c_str());

//     ScopedFile file(filename, std::ios::in | std::ios::binary);
//     std::vector<char> data(file.Size());
//     file.Read(data.data(), data.size());

//     Deserializer deserializer(std::move(data));
//     scene->Archive(deserializer);
// }

}  // namespace pine
