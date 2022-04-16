#include <pstd/algorithm.h>
#include <pstd/iostream.h>
#include <pstd/memory.h>
#include <pstd/string.h>
#include <pstd/math.h>
#include <pstd/map.h>

#include <util/format.h>
#include <util/parameters.h>

int main() {
    pine::Parameters params;

    params.Set("Hello", "World");

    pine::print(params["Hello"].GetString("type"));
}