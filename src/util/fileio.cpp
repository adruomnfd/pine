#include <core/scene.h>
#include <util/profiler.h>
#include <util/archive.h>
#include <util/huffman.h>
#include <util/fileio.h>
#include <util/parser.h>
#include <util/misc.h>
#include <util/log.h>

#include <algorithm>
#include <sstream>
#include <pstd/vector.h>

#include <ext/stb_image/stb_image_write.h>
#include <ext/stb_image/stb_image.h>

namespace pine {

pstd::string sceneDirectory = "";

ScopedFile::ScopedFile(pstd::string filename, std::ios::openmode mode) {
    filename = sceneDirectory + filename;
    std::replace(filename.begin(), filename.end(), '\\', '/');
    CHECK(filename != "");
    file.open(filename.c_str(), mode);
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

bool IsFileExist(pstd::string filename) {
    std::replace(filename.begin(), filename.end(), '\\', '/');
    std::ifstream file(filename.c_str());
    return file.good();
}
pstd::string GetFileDirectory(pstd::string filename) {
    std::replace(filename.begin(), filename.end(), '\\', '/');

    size_t forwardslash = filename.find_last_of('/');
    if (forwardslash != filename.npos)
        filename = filename.substr(0, forwardslash);

    return filename + "/";
}
pstd::string GetFileExtension(pstd::string filename) {
    size_t p = filename.find_last_of('.');
    if (p == filename.npos)
        return "";

    return filename.substr(p + 1, filename.size() - p - 1);
}
pstd::string RemoveFileExtension(pstd::string filename) {
    size_t p = filename.find_last_of('.');
    if (p == filename.npos)
        return "";

    return filename.substr(0, p);
}
pstd::string ChangeFileExtension(pstd::string filename, pstd::string ext) {
    return RemoveFileExtension(filename) + "." + ext;
}
pstd::string AppendFileName(pstd::string filename, pstd::string content) {
    return RemoveFileExtension(filename) + content + "." + GetFileExtension(filename);
}

pstd::string ReadStringFile(pstd::string filename) {
    ScopedFile file(filename, std::ios::in | std::ios::binary);
    size_t size = file.Size();
    pstd::string str;
    str.resize(size);
    file.Read(&str[0], size);
    return str;
}
void WriteBinaryData(pstd::string filename, const void *ptr, size_t size) {
    ScopedFile file(filename, std::ios::binary | std::ios::out);
    file.Write((const char *)ptr, size);
}
pstd::vector<char> ReadBinaryData(pstd::string filename) {
    ScopedFile file(filename, std::ios::binary | std::ios::in);
    pstd::vector<char> data(file.Size());
    file.Read(&data[0], file.Size());
    return data;
}

void SaveBMPImage(pstd::string filename, vec2i size, int nchannel, uint8_t *data) {
    LOG("[FileIO]Saving image \"&\"", filename);
    ScopedFile file(filename, std::ios::out);

    pstd::vector<vec3u8> colors(Area(size));
    for (int x = 0; x < size.x; x++)
        for (int y = 0; y < size.y; y++) {
            vec3u8 c;
            c.z = data[(x + y * size.x) * nchannel + 0];
            c.y = data[(x + y * size.x) * nchannel + 1];
            c.x = data[(x + y * size.x) * nchannel + 2];
            colors[x + (size.y - 1 - y) * size.x] = c;
        }

    int filesize = 54 + 3 * Area(size);
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
void SaveImage(pstd::string filename, vec2i size, int nchannel, float *data) {
    pstd::vector<uint8_t> pixels(Area(size) * nchannel);
    for (int x = 0; x < size.x; x++)
        for (int y = 0; y < size.y; y++)
            for (int c = 0; c < nchannel; c++)
                pixels[y * size.x * nchannel + x * nchannel + c] = pstd::clamp(
                    data[y * size.x * nchannel + x * nchannel + c] * 256.0f, 0.0f, 255.0f);

    SWITCH(GetFileExtension(filename)) {
        CASE("bmp") SaveBMPImage(filename, size, nchannel, (uint8_t *)pixels.data());
        CASE("png")
        stbi_write_png(filename.c_str(), size.x, size.y, nchannel, pixels.data(),
                       size.x * nchannel);
        DEFAULT
        LOG_WARNING("& has unsupported image file extension", filename);
    }
}
void SaveImage(pstd::string filename, vec2i size, int nchannel, uint8_t *data) {
    SWITCH(GetFileExtension(filename)) {
        CASE("bmp") SaveBMPImage(filename, size, nchannel, data);
        CASE("png")
        stbi_write_png(filename.c_str(), size.x, size.y, nchannel, data, size.x * nchannel);
        DEFAULT
        LOG_WARNING("& has unsupported image file extension", filename);
    }
}
vec3u8 *ReadLDRImage(pstd::string filename, vec2i &size) {
    int nchannel = 0;
    uint8_t *data =
        (uint8_t *)stbi_load((sceneDirectory + filename).c_str(), &size.x, &size.y, &nchannel, 3);
    if (!data)
        LOG_WARNING("[FileIO]Failed to load \"&\"", filename);
    return (vec3u8 *)data;
}

pstd::pair<pstd::vector<float>, vec3i> LoadVolume(pstd::string filename) {
    if (GetFileExtension(filename) == "compressed")
        return LoadCompressedVolume(filename);
    ScopedFile file(filename, std::ios::in | std::ios::binary);
    vec3i size = file.Read<vec3i>();
    pstd::vector<float> density(Volume(size));
    file.Read(&density[0], density.size() * sizeof(density[0]));

    return {pstd::move(density), size};
}
void CompressVolume(pstd::string /*filename*/, const pstd::vector<float> & /*densityf*/,
                    vec3i /*size*/) {
    // ScopedFile file(filename, std::ios::out | std::ios::binary);

    // for (size_t i = 0; i < densityf.size(); i++)
    //     if (densityf[i] > 32) {
    //         LOG_WARNING("Original density has voxel with value larger than 32, which will be "
    //                     "clamped to 32 for compression");
    //         break;
    //     }

    // pstd::vector<uint16_t> densityi(densityf.size());
    // for (size_t i = 0; i < densityf.size(); i++)
    //     densityi[i] =
    //         pstd::min(densityf[i], 32.0f) * int(pstd::numeric_limits<uint16_t>::max()) / 32;

    // auto tree = BuildHuffmanTree(densityi);
    // auto encoded = HuffmanEncode(tree, densityi);
    // auto treeData = Archive(tree);
    // auto encodedData = Archive(encoded);
    // file.Write(size);
    // file.Write(treeData.size());
    // file.Write(encodedData.size());
    // file.Write(treeData.data(), treeData.size() * sizeof(treeData[0]));
    // file.Write(encodedData.data(), encodedData.size() * sizeof(encodedData[0]));
}
pstd::pair<pstd::vector<float>, vec3i> LoadCompressedVolume(pstd::string /*filename*/) {
    // ScopedFile file(filename, std::ios::in | std::ios::binary);
    // vec3i size = file.Read<vec3i>();
    // size_t treeSize = file.Read<size_t>();
    // size_t encodedSize = file.Read<size_t>();
    // ArchiveBufferType treeData(treeSize);
    // ArchiveBufferType encodedData(encodedSize);
    // file.Read(&treeData[0], treeSize * sizeof(treeData[0]));
    // file.Read(&encodedData[0], encodedSize * sizeof(encodedData[0]));
    // auto tree = Unarchive<HuffmanTree<uint16_t>>(treeData);
    // auto encoded = Unarchive<HuffmanEncoded>(encodedData);
    // auto densityi = HuffmanDecode<pstd::vector<uint16_t>>(tree, encoded);

    // pstd::vector<float> densityf(densityi.size());
    // for (size_t i = 0; i < densityi.size(); i++)
    //     densityf[i] = densityi[i] / float(int(pstd::numeric_limits<uint16_t>::max()) / 32);

    // return {densityf, size};
    return {};
}

TriangleMesh LoadObj(pstd::string filename) {
    Profiler _("LoadObj");
    LOG_PLAIN("[FileIO]Loading \"&\"", filename);
    Timer timer;

    TriangleMesh mesh;
    pstd::string raw = ReadStringFile(filename);
    pstd::string_view str = raw;

    pstd::string face;
    face.reserve(64);

    while (true) {
        size_t pos = str.find_first_of('\n');
        if (pos == str.npos)
            break;
        pstd::string_view line = str.substr(0, pos);
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
                face += " ";
                face += line.substr(0, line.find_first_of('/'));
                line = line.substr(line.find_first_of(' ') + 1);
                face += " ";
                face += line.substr(0, line.find_first_of('/'));
            } else {
                face += line.substr(0, line.find_first_of('/'));
                line = line.substr(line.find_first_of(' ') + 1);
                face += " ";
                face += line.substr(0, line.find_first_of('/'));
                line = line.substr(line.find_first_of(' ') + 1);
                face += " ";
                face += line.substr(0, line.find_first_of('/'));
                line = line.substr(line.find_first_of(' ') + 1);
                face += " ";
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
    uint32_t minIndices = -1;
    for (auto &i : mesh.indices)
        minIndices = pstd::min(minIndices, i);
    for (auto &i : mesh.indices)
        i -= minIndices;

    LOG_PLAIN(", &M triangles, &ms\n", mesh.indices.size() / 3 / 1000000.0, timer.Reset());
    return mesh;
}

Parameters LoadScene(pstd::string filename, Scene *scene) {
    LOG("[FileIO]Loading \"&\"", filename);
    Parameters params = Parse(ReadStringFile(filename));
    sceneDirectory = GetFileDirectory(filename);

    Timer timer;

    for (auto &p : params.GetAll("Material"))
        scene->materials[p.GetString("name")] = pstd::make_shared<Material>(CreateMaterial(p));
    for (auto &p : params.GetAll("Medium"))
        scene->mediums[p.GetString("name")] = pstd::make_shared<Medium>(CreateMedium(p));
    for (auto &p : params.GetAll("Light"))
        scene->lights.push_back(CreateLight(p));
    for (auto &p : params.GetAll("Shape"))
        scene->shapes.push_back(CreateShape(p, scene));

    for (auto &shape : scene->shapes)
        if (auto light = shape.GetLight())
            scene->lights.push_back(*light);
    for (const Light &light : scene->lights)
        if (light.Is<EnvironmentLight>())
            scene->envLight = light.Be<EnvironmentLight>();

    scene->camera = CreateCamera(params["Camera"], scene);
    scene->integrator = pstd::shared_ptr<Integrator>(CreateIntegrator(params["Integrator"], scene));

    return params;
}

}  // namespace pine
