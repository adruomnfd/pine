#include <pstd/algorithm.h>
#include <pstd/iostream.h>
#include <pstd/memory.h>
#include <pstd/string.h>
#include <pstd/math.h>
#include <pstd/map.h>

#include <util/parameters.h>

#include <memory>
#include <vector>

int main() {
    pstd::vector<pstd::shared_ptr<int>> xs(5, pstd::make_shared<int>(10));

    xs.emplace_back(pstd::make_shared<int>(20));
    xs.push_back(pstd::make_shared<int>(30));

    pstd::cout << xs << pstd::endl;
}