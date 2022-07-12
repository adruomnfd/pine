#ifndef PINE_UTIL_PARALLEL_H
#define PINE_UTIL_PARALLEL_H

#include <core/vecmath.h>
#include <util/reflect.h>

#include <pstd/vector.h>
#include <thread>
#include <atomic>

namespace pine {

inline int NumThreads() {
    return std::thread::hardware_concurrency();
}

thread_local inline int threadIdx;

template <typename F, typename... Args>
void ParallelForImpl(int64_t nItems, F&& f) {
    pstd::vector<std::thread> threads(NumThreads());
    std::atomic<int64_t> i{0};
    int batchSize = pstd::max(nItems / NumThreads() / 64, (int64_t)1);
    int tid = 0;

    for (auto& thread : threads)
        thread = std::thread([&, tid = tid++]() {
            threadIdx = tid;
            while (true) {
                int64_t workId = i += batchSize;
                workId -= batchSize;

                for (int j = 0; j < batchSize; j++) {
                    if (workId + j >= nItems)
                        return;
                    f(workId + j);
                }
            }
        });

    for (auto& thread : threads)
        thread.join();
}

template <typename F, typename... Args>
void ParallelFor(int size, F&& f) {
    ParallelForImpl(size, [&f](int idx) { f(idx); });
}

template <typename F, typename... Args>
void ParallelFor(vec2i size, F&& f) {
    ParallelForImpl(Area(size), [&f, w = size.x](int idx) {
        Invoke(pstd::forward<F>(f), vec2i{idx % w, idx / w});
    });
}

struct AtomicFloat {
    explicit AtomicFloat(float v = 0) {
        bits = pstd::bitcast<uint32_t>(v);
    };
    operator float() const {
        return pstd::bitcast<float>(bits);
    }
    AtomicFloat& operator=(float v) {
        bits = pstd::bitcast<uint32_t>(v);
        return *this;
    }
    void Add(float v) {
        uint32_t oldBits = bits, newBits;
        do {
            newBits = pstd::bitcast<uint32_t>(pstd::bitcast<float>(oldBits) + v);
        } while (!bits.compare_exchange_weak(oldBits, newBits));
    }

  private:
    std::atomic<uint32_t> bits;
};

}  // namespace pine

#endif  // PINE_UTIL_PARALLEL_H