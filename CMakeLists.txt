cmake_minimum_required(VERSION 3.12)

project(pine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic -ffast-math -ffp-contract=fast")

include_directories(
    ./
    ./src
    ./ext
)

#Pine Core Library
add_library(pinelib 
src/core/vecmath.cpp
src/core/geometry.cpp
src/core/medium.cpp
src/core/light.cpp
src/core/camera.cpp
src/core/scene.cpp
src/core/accel.cpp
src/core/color.cpp
src/core/noise.cpp
src/core/node.cpp
src/core/bxdf.cpp
src/core/material.cpp
src/core/integrator.cpp
src/util/log.cpp
src/util/string.cpp
src/util/fileio.cpp
src/util/parser.cpp
src/util/profiler.cpp
src/util/parameters.cpp
src/impl/integrators/ao.cpp
src/impl/integrators/path.cpp
src/impl/integrators/photon.cpp
src/impl/accel/bvh.cpp
src/impl/accel/cwbvh.cpp
)

set_target_properties(pinelib PROPERTIES POSITION_INDEPENDENT_CODE ON)
if(UNIX)
target_link_libraries(pinelib pthread)
endif()

#Pine CommandLine
add_executable(pine src/cmd/main.cpp)
target_link_libraries(pine pinelib)
