#ifndef PINE_UTIL_PROFILER_H
#define PINE_UTIL_PROFILER_H

#include <util/log.h>

#include <map>
#include <memory>

namespace pine {

struct Profiler {
    static void ReportStat();

    Profiler(std::string description);
    ~Profiler();
    PINE_DELETE_COPY_MOVE(Profiler)

    struct Record {
        friend bool operator==(const Record& lhs, const Record& rhs) {
            return lhs.time == rhs.time && lhs.sampleCount == rhs.sampleCount &&
                   lhs.name == rhs.name;
        }

        std::shared_ptr<Record> parent;
        std::map<std::string, std::shared_ptr<Record>> children;
        std::string name;
        double time = 0.0f;
        int sampleCount = 0;
    };

    Timer timer;
};

enum class ProfilePhase {
    GenerateRay,
    ShapeIntersect,
    BoundingBoxIntersect,
    IntersectClosest,
    IntersectShadow,
    IntersectTr,
    MaterialSample,
    EstimateDirect,
    EstimateLi,
    MediumTr,
    MediumSample,
    SearchNeighbors,
    GenerateSamples,
    NumPhase
};
static const char* profilePhaseName[] __attribute__((unused)) = {
    "GenerateRay",     "ShapeIntersect", "BoundingBoxIntersect", "IntersectClosest",
    "IntersectShadow", "IntersectTr",    "MaterialSample",       "EstimateDirect",
    "EstimateLi",      "MediumTr",       "MediumSample",         "SearchNeighbors",
    "GenerateSamples"
};

struct SampledProfiler {
    static void Initialize();
    static void ReportStat();

    SampledProfiler(ProfilePhase phase);
    ~SampledProfiler();

    ProfilePhase p;
};

}  // namespace pine

#endif  // PINE_UTIL_PROFILER_H