cmake_minimum_required(VERSION 3.12)

project(pine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(-Wall -Wextra -pedantic -ffast-math -ffp-contract=fast)
# add_compile_options(-Wall -Wextra -pedantic -ffast-math -ffp-contract=fast -O0 -g)
# add_compile_options(-Wall -Wextra -pedantic -O0 -g -fsanitize=address)
# add_compile_options(-Wall -Wextra -pedantic -O0 -g)
# add_link_options(-fsanitize=address)

include_directories(
    ./
    ./src
    ./ext
)

#pstd Library
add_library(pstd 
src/pstd/string.cpp
src/pstd/chrono.cpp
src/pstd/fstream.cpp
src/pstd/iostream.cpp
src/pstd/stacktrace.cpp
)

#Pine Core Library
add_library(pinelib 
src/core/geometry.cpp
src/core/medium.cpp
src/core/spectrum.cpp
src/core/light.cpp
src/core/camera.cpp
src/core/accel.cpp
src/core/color.cpp
src/core/noise.cpp
src/core/film.cpp
src/core/filter.cpp
src/core/node.cpp
src/core/bxdf.cpp
src/core/lowdiscrepancy.cpp
src/core/lightsampler.cpp
src/core/sampler.cpp
src/core/material.cpp
src/core/integrator.cpp
src/util/log.cpp
src/util/primes.cpp
src/util/sobolmetrices.cpp
src/util/format.cpp
src/util/fileio.cpp
src/util/parser.cpp
src/util/profiler.cpp
src/util/objloader.cpp
src/util/parameters.cpp
src/impl/integrator/ao.cpp
src/impl/integrator/viz.cpp
src/impl/integrator/mlt.cpp
src/impl/integrator/path.cpp
src/impl/integrator/area.cpp
src/impl/integrator/bdpt.cpp
src/impl/integrator/sppm.cpp
src/impl/integrator/lightpath.cpp
src/impl/integrator/randomwalk.cpp
src/impl/accel/bvh.cpp
src/impl/accel/cwbvh.cpp
ext/stb_image/stb_image.cpp
ext/stb_image/stb_image_write.cpp
)

set_target_properties(pinelib PROPERTIES POSITION_INDEPENDENT_CODE ON)
find_package(Threads REQUIRED)
target_link_libraries(pinelib pstd Threads::Threads)

#Pine CommandLine
add_executable(pine src/cli/main.cpp)
target_link_libraries(pine pinelib)

# #Image Tool
# add_executable(imgtool src/cli/imgtool.cpp)
# target_link_libraries(imgtool pinelib)

# #Volume Tool
# add_executable(voltool src/cli/voltool.cpp)
# target_link_libraries(voltool pinelib)

# #Pine Test
add_executable(pine_test test/test_main.cpp)
target_link_libraries(pine_test pinelib)

#Pstd Test
add_executable(pstd_test test/pstd_test.cpp)
target_link_libraries(pstd_test pinelib)
