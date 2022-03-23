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
    // auto putback = [&]() {
    //     ++argc;
    //     --argv;
    // };
    auto files = [&]() { return std::vector<std::string>(argv, argv + argc); };

    // clang-format off

    SWITCH(next()) {
        CASE("compress")
                if(argc == 1){
                    auto [density, size] = LoadVolume(files()[0]);
                    CompressVolume(ChangeFileExtension(files()[0], "compressed"), density, size);
                }
                else{
                    LOG("Usage: compress [from] [to]");
                }       
        DEFAULT
            LOG("Usage: voltool [compress] [filename]");
    }

    // clang-format on

    return 0;
}