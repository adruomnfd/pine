#ifndef PINE_UTIL_PARALLEL_H
#define PINE_UTIL_PARALLEL_H

#include <core/vecmath.h>

#include <vector>
#include <thread>
#include <atomic>

namespace pine {

inline int NumThreads() {
    return std::thread::hardware_concurrency();
}

template <typename F, typename... Args>
void ParallelForImpl(int64_t nItems, F&& f) {
    std::vector<std::thread> threads(NumThreads());
    std::atomic<int64_t> i{0};
    int batchSize = max(nItems / NumThreads() / 64, (int64_t)1);
    int threadId = 0;

    for (auto& thread : threads)
        thread = std::thread([&, threadId = threadId++]() {
            while (true) {
                int64_t workId = i += batchSize;
                workId -= batchSize;

                for (int j = 0; j < batchSize; j++) {
                    if (workId + j >= nItems)
                        return;
                    f(threadId, workId + j);
                }
            }
        });

    for (auto& thread : threads)
        thread.join();
}

template <typename F, typename... Args>
void ParallelFor(int size, F&& f) {
    ParallelForImpl(size, [&f](int, int index) { f(index); });
}

template <typename F, typename... Args>
void ParallelFor(vec2i size, F&& f) {
    ParallelForImpl(size.x * size.y, [&f, w = size.x](int, int index) {
        f({index % w, index / w});
    });
}

template <typename F, typename... Args>
void ThreadIdParallelFor(int size, F&& f) {
    ParallelForImpl(size, f);
}

template <typename F, typename... Args>
void ThreadIdParallelFor(vec2i size, F&& f) {
    ParallelForImpl(size.x * size.y, [&f, w = size.x](int threadId, int index) {
        f(threadId, {index % w, index / w});
    });
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