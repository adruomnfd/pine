#include <pstd/stacktrace.h>

#include <execinfo.h>
#include <cxxabi.h>

namespace pstd {

 pstd::string stacktrace() {
    pstd::string str = "stack trace\n";

    constexpr int MaxFrames = 64;
    void* addresses[MaxFrames] = {};

    int addrlen = backtrace(addresses, MaxFrames);

    if (addrlen == 0) {
        str += "  empty, possibly corruption\n";
    } else {
        char** symbols = backtrace_symbols(addresses, addrlen);

        for (int i = 1; i < addrlen; ++i)
            str += "  " + pstd::string(symbols[i]) + "\n";

        free(symbols);
    }

    return str;
}

}  // namespace pstd