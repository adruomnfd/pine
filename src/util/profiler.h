#ifndef PINE_UTIL_PROFILER_H
#define PINE_UTIL_PROFILER_H

#include <util/log.h>

#include <pstd/map.h>
#include <pstd/memory.h>

namespace pine {

struct Profiler {
    static void Initialize() {
        main = pstd::unique_ptr<Profiler>(new Profiler("Main"));
    }
    static void Finalize();

    Profiler(pstd::string description);
    ~Profiler();
    PINE_DELETE_COPY_MOVE(Profiler)

    struct Record {
        friend bool operator==(const Record& lhs, const Record& rhs) {
            return lhs.time == rhs.time && lhs.sampleCount == rhs.sampleCount &&
                   lhs.name == rhs.name;
        }

        pstd::shared_ptr<Record> parent;
        pstd::map<pstd::string, pstd::shared_ptr<Record>> children;
        pstd::string name;
        double time = 0.0f;
        int sampleCount = 0;
    };

    Timer timer;

    static inline pstd::unique_ptr<Profiler> main;
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
    FilmAddSample,
    SampleEnvLight,
    NumPhase
};
static const char* profilePhaseName[] = {
    "GenerateRay",     "ShapeIntersect", "BoundingBoxIntersect", "IntersectClosest",
    "IntersectShadow", "IntersectTr",    "MaterialSample",       "EstimateDirect",
    "EstimateLi",      "MediumTr",       "MediumSample",         "SearchNeighbors",
    "GenerateSamples", "FilmAddSample",  "SampleEnvLight"};

struct SampledProfiler {
    static void Initialize();
    static void Finalize();

    SampledProfiler(ProfilePhase phase);
    ~SampledProfiler();

    ProfilePhase p;
};

}  // namespace pine

#endif  // PINE_UTIL_PROFILER_H