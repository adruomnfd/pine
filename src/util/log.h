#ifndef PINE_UTIL_LOG_H
#define PINE_UTIL_LOG_H

#include <util/string.h>

#include <chrono>

namespace pine {

extern bool verbose;

template <typename... Args>
inline void LOG(const Args&... args) {
    printf("%s\n", Fstring(args...).c_str());
}
template <typename... Args>
inline void LOG_SAMELINE(const Args&... args) {
    printf("\33[2K\r%s\r", Fstring(args...).c_str());
    fflush(stdout);
}
template <typename... Args>
inline void LOG_VERBOSE(const Args&... args) {
    if (!verbose)
        return;
    printf("%s\n", Fstring(args...).c_str());
}
template <typename... Args>
inline void LOG_VERBOSE_SAMELINE(const Args&... args) {
    if (!verbose)
        return;
    printf("\r\33[2K%s\r", Fstring(args...).c_str());
    fflush(stdout);
}
template <typename... Args>
inline void LOG_WARNING(const Args&... args) {
    printf("\033[1;33m%s\033[0m\n", Fstring(args...).c_str());
}

template <typename... Args>
inline void LOG_FATAL(const Args&... args) {
    printf("\033[1;31m%s\033[0m\n", Fstring(args...).c_str());
    abort();
}

template <typename... Args>
inline void print(const Args&... args) {
    std::string format;
    for (size_t i = 0; i < sizeof...(args); i++)
        format += "& ";
    LOG(format.c_str(), args...);
}

#define CHECK(x)                                                                          \
    if (!(x)) {                                                                           \
        printf("\033[1;31m[CHECK Failure]%s failed[file %s, line %d, %s()]\033[0m\n", #x, \
               __FILE__, __LINE__, __func__);                                             \
        abort();                                                                          \
    }
#define CHECK_EQ(a, b)                                                                            \
    if (!((a) == (b)))                                                                            \
        LOG_FATAL("[CHECK_EQ Failure]with & equal &, & equal & [file &, line &, &()]", #a, a, #b, \
                  b, __FILE__, __LINE__, __func__);
#define CHECK_NE(a, b)                                                                            \
    if (!((a) != (b)))                                                                            \
        LOG_FATAL("[CHECK_NE Failure]with & equal &, & equal & [file &, line &, &()]", #a, a, #b, \
                  b, __FILE__, __LINE__, __func__);
#define CHECK_LT(a, b)                                                                            \
    if (!((a) < (b)))                                                                             \
        LOG_FATAL("[CHECK_LT Failure]with & equal &, & equal & [file &, line &, &()]", #a, a, #b, \
                  b, __FILE__, __LINE__, __func__);
#define CHECK_GT(a, b)                                                                            \
    if (!((a) > (b)))                                                                             \
        LOG_FATAL("[CHECK_GT Failure]with & equal &, & equal & [file &, line &, &()]", #a, a, #b, \
                  b, __FILE__, __LINE__, __func__);
#define CHECK_LE(a, b)                                                                            \
    if (!((a) <= (b)))                                                                            \
        LOG_FATAL("[CHECK_LE Failure]with & equal &, & equal & [file &, line &, &()]", #a, a, #b, \
                  b, __FILE__, __LINE__, __func__);
#define CHECK_GE(a, b)                                                                            \
    if (!((a) >= (b)))                                                                            \
        LOG_FATAL("[CHECK_GE Failure]with & equal &, & equal & [file &, line &, &()]", #a, a, #b, \
                  b, __FILE__, __LINE__, __func__);

#ifndef NDEBUG
#define DCHECK(x) CHECK(x)
#define DCHECK_EQ(a, b) CHECK_EQ(a, b)
#define DCHECK_NE(a, b) CHECK_NE(a, b)
#define DCHECK_LT(a, b) CHECK_LT(a, b)
#define DCHECK_GT(a, b) CHECK_GT(a, b)
#define DCHECK_LE(a, b) CHECK_LE(a, b)
#define DCHECK_GE(a, b) CHECK_GE(a, b)
#else
#define DCHECK(x)
#define DCHECK_EQ(a, b)
#define DCHECK_NE(a, b)
#define DCHECK_LT(a, b)
#define DCHECK_GT(a, b)
#define DCHECK_LE(a, b)
#define DCHECK_GE(a, b)
#endif

struct Timer {
    using clock = std::chrono::high_resolution_clock;
    double ElapsedMs();
    double Reset();
    clock::time_point start = clock::now();
};

struct ProgressReporter {
    ProgressReporter() = default;
    ProgressReporter(std::string tag, std::string desc, std::string performance, int64_t total,
                     int64_t multiplier = 1)
        : tag(tag), desc(desc), performance(performance), multiplier(multiplier), total(total) {
    }

    void Report(int64_t current);

  private:
    std::string tag, desc, performance;
    Timer ETA, interval;
    int64_t multiplier, previous;

  public:
    int64_t total;
};

struct ScopedPR {
    ScopedPR(ProgressReporter& pr, int64_t current, bool lastIter = false)
        : pr(pr), lastIter(lastIter) {
        pr.Report(current);
    }
    ~ScopedPR() {
        if (lastIter)
            pr.Report(pr.total);
    }
    PINE_DELETE_COPY_MOVE(ScopedPR)

    ProgressReporter& pr;
    bool lastIter;
};

}  // namespace pine

#endif  // PINE_UTIL_LOG_H