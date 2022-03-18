#include <util/fileio.h>

using namespace pine;

static void convertFormat(std::vector<std::string> filenames, std::string newFormat, bool rm) {
    size_t maxLen = 0;
    for (auto& filename : filenames)
        maxLen = std::max(filename.size(), maxLen);
    for (auto& from : filenames) {
        std::string to = from.substr(0, from.find_last_of('.') + 1) + newFormat;
        LOG("&      ======>      &", Format(maxLen), from, to);
        vec2i size;
        std::unique_ptr<vec3u8[]> data(ReadLDRImage(from, size));

        if (rm)
            remove(from.c_str());
        SaveImage(to, size, 3, (uint8_t*)data.get());
    }
};

int main(int argc, char* argv[]) {
    --argc;
    ++argv;

    auto next = [&]() -> std::string {
        if (argc == 0)
            return "";
        else {
            argc--;
            return *(argv++);
        }
    };

    SWITCH(next()) {
        CASE("convert") {
            SWITCH(next()) {
                CASE("--inplace" && argc > 2)
                auto fmt = next();
                convertFormat(std::vector<std::string>(argv, argv + argc), fmt, true);
                DEFAULT
                LOG("convert [--inplace] [format] [filename]...");
            }
        }
        DEFAULT {
            LOG("Usage: imgtool [convert] [filename]...");
        }
    }

    return 0;
}