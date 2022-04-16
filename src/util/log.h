#ifndef PINE_UTIL_LOG_H
#define PINE_UTIL_LOG_H

#include <util/format.h>

#include <pstd/chrono.h>
#include <pstd/iostream.h>
#include <pstd/stacktrace.h>

namespace pine {

template <typename... Args>
inline void LOG_PLAIN(const Args&... args) {
    pstd::cout << FormatIt(args...);
}
template <typename... Args>
inline void LOG(const Args&... args) {
    pstd::cout << FormatIt(args...) << pstd::endl;
}
template <typename... Args>
inline void LOG_SAMELINE(const Args&... args) {
    pstd::cout << "\33[2K\r" << FormatIt(args...) << "\r";
}
template <typename... Args>
inline void LOG_WARNING(const Args&... args) {
    pstd::cout << "\033[1;33m" << FormatIt(args...) << "\033[0m\n" << pstd::endl;
}

template <typename... Args>
inline void LOG_FATAL(const Args&... args) {
    pstd::cout << "\033[1;31m" << FormatIt(args...) << "\033[0m\n" << pstd::endl;
    pstd::cout << "\033[1;33m" << pstd::stacktrace() << "\033[0m\n" << pstd::endl;
    abort();
}

template <typename... Args>
inline void print(const Args&... args) {
    pstd::string format;
    for (size_t i = 0; i < sizeof...(args); i++)
        format += "& ";
    pstd::cout << FormatIt(format, args...) << pstd::endl;
}

#define CHECK(x)                                                                         \
    if (!(x)) {                                                                          \
        LOG_FATAL("[CHECK Failure]& failed[&:&:&()]", #x, __FILE__, __LINE__, __func__); \
        abort();                                                                         \
    }

#define CHECK_IMPL(name, op, a, b)                                                                 \
    if (!((a)op(b)))                                                                               \
        LOG_FATAL("[" name " Failure]with & equal &, & equal & [&:&:&()]", #a, a, #b, b, __FILE__, \
                  __LINE__, __func__);

#define CHECK_EQ(a, b) CHECK_IMPL("CHECK_EQ", ==, a, b)
#define CHECK_NE(a, b) CHECK_IMPL("CHECK_NE", !=, a, b)
#define CHECK_LT(a, b) CHECK_IMPL("CHECK_LT", <, a, b)
#define CHECK_GT(a, b) CHECK_IMPL("CHECK_GT", >, a, b)
#define CHECK_LE(a, b) CHECK_IMPL("CHECK_LE", <=, a, b)
#define CHECK_GE(a, b) CHECK_IMPL("CHECK_GE", >=, a, b)

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
    double ElapsedMs();
    double Reset();

  private:
    pstd::clock clock;
    float t0 = clock.now();
};

struct ProgressReporter {
    ProgressReporter() = default;
    ProgressReporter(pstd::string tag, pstd::string desc, pstd::string performance, int64_t total,
                     int64_t multiplier = 1)
        : tag(tag), desc(desc), performance(performance), multiplier(multiplier), total(total) {
    }

    void Report(int64_t current);

  private:
    pstd::string tag, desc, performance;
    Timer ETA, interval;
    int64_t multiplier = 1, previous = 0;

  public:
    int64_t total;
};

struct ScopedPR {
    ScopedPR(ProgressReporter& pr, int64_t current, bool lastIter = false, bool active = true)
        : pr(pr), lastIter(lastIter) {
        if (active)
            pr.Report(current);
    }
    ~ScopedPR() {
        if (lastIter)
            pr.Report(pr.total);
    }
    PINE_DELETE_COPY_MOVE(ScopedPR)

    ProgressReporter& pr;
    bool lastIter;
    bool active;
};

}  // namespace pine

#endif  // PINE_UTIL_LOG_H