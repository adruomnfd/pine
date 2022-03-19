#include <util/fileio.h>

using namespace pine;

size_t MaxLength(const std::vector<std::string>& names) {
    size_t maxLen = 0;
    for (auto& name : names)
        maxLen = std::max(name.size(), maxLen);
    return maxLen;
}

static void ConvertFormat(const std::vector<std::string>& filenames, std::string newFormat,
                          bool inplace) {
    size_t maxLen = MaxLength(filenames);
    for (auto& from : filenames) {
        std::string to = ChangeFileExtension(from, newFormat);
        LOG("&      ======>      &", Format(maxLen), from, to);
        vec2i size;
        std::unique_ptr<vec3u8[]> data(ReadLDRImage(from, size));

        if (inplace)
            remove(from.c_str());
        SaveImage(to, size, 3, (uint8_t*)data.get());
    }
};

static void Scaling(const std::vector<std::string>& filenames, float scale, bool inplace) {
    size_t maxLen = MaxLength(filenames);
    for (auto& from : filenames) {
        std::string to = inplace ? from : AppendFileName(from, Fstring("_x&", scale));
        LOG("&      ===x&==>      &", Format(maxLen), from, scale, to);
        vec2i size;
        std::unique_ptr<vec3u8[]> data(ReadLDRImage(from, size));
        vec2i scaledSize = size * scale;
        std::unique_ptr<vec3u8[]> scaled(new vec3u8[scaledSize.x * scaledSize.y]);
        for (int y = 0; y < size.y; y++)
            for (int x = 0; x < size.x; x++) {
                int ix = min(x * scale, scaledSize.x - 1.0f);
                int iy = min(y * scale, scaledSize.y - 1.0f);
                scaled[iy * scaledSize.x + ix] = data[y * size.x + x];
            }
        SaveImage(to, scaledSize, 3, (uint8_t*)scaled.get());
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
    auto putback = [&]() {
        ++argc;
        --argv;
    };
    auto files = [&]() { return std::vector<std::string>(argv, argv + argc); };

    // clang-format off

    SWITCH(next()) {
        CASE("convert")
            auto fmt = next();
            SWITCH(next()) {
                CASE("--inplace" && argc)
                    ConvertFormat(files(), fmt, true);
                CASE_BEGINWITH("--")
                    LOG("convert [bmp | png] [--inplace] [filename]...");
                DEFAULT
                    putback();
                    ConvertFormat(files(), fmt, false);
            }
        CASE("scaling")
            float scale = std::stof(next());
            SWITCH(next()){
                CASE("--inplace") 
                    Scaling(files(), scale, true);
                CASE_BEGINWITH("--")
                    LOG("scaling [scale] [--inplace] [filename]...");
                DEFAULT 
                    putback();
                    Scaling(files(), scale, false);
            }           
        DEFAULT
            LOG("Usage: imgtool [convert | scaling] [filename]...");
    }

    // clang-format on

    return 0;
}