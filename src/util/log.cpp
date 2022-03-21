#include <util/log.h>

#include <iostream>
#include <chrono>
#include <mutex>
#include <cmath>

namespace pine {

bool verbose = true;

double Timer::ElapsedMs() {
    return std::chrono::duration<double>(clock::now() - start).count() * 1000.0;
}
double Timer::Reset() {
    double elapsed = ElapsedMs();
    start = clock::now();
    return elapsed;
}

void ProgressReporter::Report(int64_t current) {
    static std::mutex mutex;
    if (current < previous)
        return;
    std::lock_guard<std::mutex> lk(mutex);
    int nDigit = std::max((int)log10(total) + 1, 1);
    if (current == 0) {
        ETA.Reset();
        interval.Reset();
        LOG_SAMELINE("[&]&[0/&]  Progress[0%]  ETA[?] ?M &/s", tag, desc, Format(nDigit), total,
                     performance);
    } else {
        LOG_SAMELINE("[&]&[&/&]  Progress[&2.1%]  ETA[&.0s] &3.3M &/s", tag, desc, Format(nDigit),
                     current, Format(nDigit), total, 100.0 * current / total,
                     std::ceil((total - current) * ETA.ElapsedMs() / (1000.0 * current)),
                     (current - previous) * multiplier / (interval.Reset() * 1000.0f), performance);
    }
    if (current == total)
        LOG_SAMELINE("[&]Average:&4.4M &/s\n", tag, total * multiplier / (ETA.ElapsedMs() * 1000.0),
                     performance);
    previous = current;
}

}  // namespace pine