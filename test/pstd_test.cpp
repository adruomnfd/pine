#include <pstd/algorithm.h>
#include <pstd/iostream.h>
#include <pstd/memory.h>
#include <pstd/string.h>
#include <pstd/map.h>

#include <map>

int main() {
    pstd::map<pstd::string, int> x;

    x["Hello"] = 10;
    x["World"] = 17;
    x["Time"] = 17;

    pstd::cout << x << pstd::endl;
}