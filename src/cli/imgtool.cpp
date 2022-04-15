#include <util/fileio.h>
#include <util/huffman.h>

using namespace pine;

size_t MaxLength(const pstd::vector<pstd::string>& names) {
    size_t maxLen = 0;
    for (auto& name : names)
        maxLen = pstd::max(name.size(), maxLen);
    return maxLen;
}

static void ConvertFormat(const pstd::vector<pstd::string>& filenames, pstd::string newFormat,
                          bool inplace) {
    size_t maxLen = MaxLength(filenames);
    for (auto& from : filenames) {
        pstd::string to = ChangeFileExtension(from, newFormat);
        LOG("&      ======>      &", Format(maxLen), from, to);
        vec2i size;
        pstd::unique_ptr<vec3u8[]> data(ReadLDRImage(from, size));

        if (inplace)
            remove(from.c_str());
        SaveImage(to, size, 3, (uint8_t*)data.get());
    }
};

static void Scaling(const pstd::vector<pstd::string>& filenames, float scale, bool inplace) {
    size_t maxLen = MaxLength(filenames);
    for (auto& from : filenames) {
        pstd::string to = inplace ? from : AppendFileName(from, Fstring("_x&", scale));
        LOG("&      ===x&==>      &", Format(maxLen), from, scale, to);
        vec2i size;
        pstd::unique_ptr<vec3u8[]> data(ReadLDRImage(from, size));
        vec2i scaledSize = size * scale;
        pstd::unique_ptr<vec3u8[]> scaled(new vec3u8[scaledSize.x * scaledSize.y]);
        for (int y = 0; y < size.y; y++)
            for (int x = 0; x < size.x; x++) {
                int ix = min(x * scale, scaledSize.x - 1.0f);
                int iy = min(y * scale, scaledSize.y - 1.0f);
                scaled[iy * scaledSize.x + ix] = data[y * size.x + x];
            }
        SaveImage(to, scaledSize, 3, (uint8_t*)scaled.get());
    }
};

static void Compress(pstd::string from, pstd::string to) {
    auto input = ReadBinaryData(from);
    auto tree = BuildHuffmanTree(input);
    auto encoded = HuffmanEncode(tree, input);
    auto serialized = Archive(tree, encoded);

    WriteBinaryData(to, serialized.data(), sizeof(serialized[0]) * serialized.size());
}

static void Decompress(pstd::string from, pstd::string to) {
    auto serialized = ReadBinaryData(from);
    auto [tree, encoded] = Unarchive<HuffmanTree<uint8_t>, HuffmanEncoded>(serialized);
    auto input = HuffmanDecode<pstd::vector<uint8_t>>(tree, encoded);

    WriteBinaryData(to, input.data(), sizeof(input[0]) * input.size());
}

int main(int argc, char* argv[]) {
    --argc;
    ++argv;

    auto lookahead = [&]() -> pstd::string {
        if (argc == 0)
            return "";
        else
            return *argv;
    };
    auto next = [&]() -> pstd::string {
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
    auto isnumber = [&]() {
        try {
            pstd::stof(lookahead());
        } catch (...) {
            return false;
        }
        return true;
    };
    auto files = [&]() { return pstd::vector<pstd::string>(argv, argv + argc); };

    auto usage = [](auto msg) {
        LOG(msg);
        exit(0);
    };

    // clang-format off

    SWITCH(next()) {
        CASE("convert")
            auto fmt = next();
            SWITCH(next()) {
                CASE("--inplace" && argc)
                    ConvertFormat(files(), fmt, true);
                CASE_BEGINWITH("--")
                    usage("convert [bmp | png] [--inplace] [filename]...");
                DEFAULT
                    putback();
                    ConvertFormat(files(), fmt, false);
            }
        CASE("scaling")
            if(!isnumber())
                usage("scaling [scale] [--inplace] [filename]...");
            float scale = pstd::stof(next());
            SWITCH(next()){
                CASE("--inplace") 
                    Scaling(files(), scale, true);
                CASE_BEGINWITH("--")
                    usage("scaling [scale] [--inplace] [filename]...");
                DEFAULT 
                    putback();
                    Scaling(files(), scale, false);
            }
        CASE("compress")
            if(files().size() != 2)  
                usage("compress [from] [to]");
            Compress(files()[0],files()[1]);         
        CASE("decompress")
            if(files().size() != 2)  
                usage("compress [from] [to]");
            Decompress(files()[0],files()[1]);         
        DEFAULT
            usage("Usage: imgtool [convert | scaling] [filename]...");
    }

    // clang-format on

    return 0;
}