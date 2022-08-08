#include <pstd/system.h>
#include <pstd/string.h>

//#include <execinfo.h>
//#include <cxxabi.h>
//#include <stdlib.h>

namespace pstd {

void abort() {
    ::abort();
}

string stacktrace() {
    return "";
    /*
    string str = "stack trace\n";

    constexpr int MaxFrames = 64;
    void* addresses[MaxFrames] = {};

    int addrlen = backtrace(addresses, MaxFrames);

    if (addrlen == 0) {
        str += "  <empty, possibly corruption>\n";
    } else {
        char** symbols = backtrace_symbols(addresses, addrlen);

        for (int i = 1; i < addrlen; ++i) {
            string name = symbols[i];
            str += name;
            auto p0 = pstd::find(pstd::begin(name), pstd::end(name), '(') - pstd::begin(name);
            auto p1 = pstd::find(pstd::begin(name), pstd::end(name), '+') - pstd::begin(name);
            auto p2 = pstd::find(pstd::begin(name) + p1, pstd::end(name), ')') - pstd::begin(name);
            name = pstd::trim(name, p0 + 1, p2 - p0 - 1);
            name[p1 - p0 - 1] = '\0';

            int status = 0;
            name = abi::__cxa_demangle(name.c_str(), nullptr, nullptr, &status);

            if (status != 0)
                str += "  <fail to demangle>\n";
            else
                str += "  " + name + "\n";
        }

        free(symbols);
    }

    return str;
    */
}

}  // namespace pstd
