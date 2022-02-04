#ifndef PINE_UTIL_PARALLEL_H
#define PINE_UTIL_PARALLEL_H

#include <core/vecmath.h>

#include <vector>
#include <thread>
#include <atomic>

namespace pine {

template <typename F>
void ParallelFor(int64_t nItems, F f) {
    std::vector<std::thread> threads(std::thread::hardware_concurrency());
    std::atomic<int64_t> i{0};

    for (auto& thread : threads)
        thread = std::thread([&]() {
            int64_t workId = i++;
            while (workId < nItems) {
                f(workId);
                workId = i++;
            }
        });

    for (auto& thread : threads)
        thread.join();
}

template <typename F>
void ParallelFor(vec2i size, F f) {
    std::vector<std::thread> threads(std::thread::hardware_concurrency());
    std::atomic<int64_t> y{0};

    for (auto& thread : threads)
        thread = std::thread([&]() {
            int workId = y++;
            while (workId < size.y) {
                for (int x = 0; x < size.x; x++)
                    f({x, workId});
                workId = y++;
            }
        });

    for (auto& thread : threads)
        thread.join();
}

template <typename F>
void For(int64_t nItems, F f) {
    for (int64_t i = 0; i < nItems; i++)
        f(i);
}
template <typename F>
void For(vec2i size, F f) {
    for (int y = 0; y < size.y; y++)
        for (int x = 0; x < size.x; x++)
            f({x, y});
}

}  // namespace pine

#endif  // PINE_UTIL_PARALLEL_H