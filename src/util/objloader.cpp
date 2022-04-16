#include <core/geometry.h>
#include <core/vecmath.h>
#include <util/objloader.h>
#include <util/profiler.h>
#include <util/string.h>
#include <util/fileio.h>

#include <pstd/algorithm.h>

namespace pine {

TriangleMesh LoadObj(pstd::string_view filename) {
    Profiler _("LoadObj");
    LOG_PLAIN("[FileIO]Loading \"&\"", filename);
    Timer timer;

    TriangleMesh mesh;
    pstd::string raw = ReadStringFile(filename);
    pstd::string_view str = raw;

    pstd::string face;
    face.reserve(64);

    while (true) {
        size_t pos = pstd::find_first_of(begin(str), end(str), '\n') - begin(str);
        if (pstd::find_first_of(begin(str), end(str), '\n') == pstd::end(str))
            break;
        pstd::string_view line = trim(str, 0, pos);
        if (line[0] == 'v' && line[1] != 't' && line[1] != 'n') {
            vec3 v;
            StrToFloats((pstd::string)trim(line, 2), &v[0], 3);
            mesh.vertices.push_back(v);

        } else if (line[0] == 'f') {
            line = trim(line, 2);
            size_t forwardSlashCount = pstd::count(begin(line), end(line), '/');
            if (forwardSlashCount == 0) {
                face = line;
            } else if (forwardSlashCount == 6) {
                face +=
                    trim(line, 0, pstd::find_first_of(begin(line), end(line), '/') - begin(line));
                line =
                    trim(line, pstd::find_first_of(begin(line), end(line), ' ') - begin(line) + 1);
                face += " ";
                face +=
                    trim(line, 0, pstd::find_first_of(begin(line), end(line), '/') - begin(line));
                line =
                    trim(line, pstd::find_first_of(begin(line), end(line), ' ') - begin(line) + 1);
                face += " ";
                face +=
                    trim(line, 0, pstd::find_first_of(begin(line), end(line), '/') - begin(line));
            } else {
                face +=
                    trim(line, 0, pstd::find_first_of(begin(line), end(line), '/') - begin(line));
                line =
                    trim(line, pstd::find_first_of(begin(line), end(line), ' ') - begin(line) + 1);
                face += " ";
                face +=
                    trim(line, 0, pstd::find_first_of(begin(line), end(line), '/') - begin(line));
                line =
                    trim(line, pstd::find_first_of(begin(line), end(line), ' ') - begin(line) + 1);
                face += " ";
                face +=
                    trim(line, 0, pstd::find_first_of(begin(line), end(line), '/') - begin(line));
                line =
                    trim(line, pstd::find_first_of(begin(line), end(line), ' ') - begin(line) + 1);
                face += " ";
                face +=
                    trim(line, 0, pstd::find_first_of(begin(line), end(line), '/') - begin(line));
            }

            size_t spaceCount = pstd::count(begin(face), end(face), ' ');
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
        str = trim(str, pos + 1);
    }
    uint32_t minIndices = -1;
    for (auto &i : mesh.indices)
        minIndices = pstd::min(minIndices, i);
    for (auto &i : mesh.indices)
        i -= minIndices;

    LOG_PLAIN(", &M triangles, &ms\n", mesh.indices.size() / 3 / 1000000.0, timer.Reset());
    return mesh;
}

}  // namespace pine