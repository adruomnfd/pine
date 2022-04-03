#include <util/fileio.h>

using namespace pine;

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
    auto files = [&]() { return std::vector<std::string>(argv, argv + argc); };
    auto usage = [](auto msg) {
        LOG(msg);
        exit(0);
    };

    // clang-format off

    SWITCH(next()) {
        CASE("compress")
            if(argc == 1){
                auto [density, size] = LoadVolume(files()[0]);
                CompressVolume(ChangeFileExtension(files()[0], "compressed"), density, size);
            }
            else{
                usage("Usage: compress [from] [to]");
            }       
        DEFAULT
            usage("Usage: voltool [compress] [filename]");
    }

    // clang-format on

    return 0;
}