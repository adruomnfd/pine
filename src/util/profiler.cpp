#include <util/profiler.h>

#include <pstd/map.h>
#include <pstd/vector.h>
#include <pstd/memory.h>

#include <mutex>
#include <atomic>

#include <signal.h>
#include <sys/time.h>

namespace pine {

static pstd::shared_ptr<Profiler::Record> profilerRecord = pstd::make_shared<Profiler::Record>();

void Profiler::Finalize() {
    main.reset();
    LOG("[Profiler]Results:");

    LOG("#structured:");
    auto ReportRecord = [](auto& me, Record record, size_t indent, double totalTime) -> void {
        if (totalTime != 0.0f && record.time / totalTime < 0.005f)
            return;
        if (record.name != "") {
            LOG("|& &< &9 calls &10.1 ms    &3.2 %", Format(indent), "", Format(40 - indent),
                record.name.c_str(), record.sampleCount, record.time,
                (totalTime == 0.0f) ? 100.0 : 100.0 * record.time / totalTime);
            indent += 2;
        }

        pstd::vector<Record> sorted;
        for (auto& rec : record.children)
            sorted.push_back(*rec.second);
        pstd::sort(sorted,
                   [](const Record& lhs, const Record& rhs) { return lhs.time > rhs.time; });
        for (auto& rec : sorted)
            me(me, rec, indent, record.time);
    };
    ReportRecord(ReportRecord, *profilerRecord, 0, 0.0f);

    LOG("\n#flattened:");
    pstd::vector<Record> flat;
    auto Flatten = [&](auto& me, const auto& map, pstd::vector<pstd::string> parents) -> void {
        for (auto& rec : map) {
            int existPos = -1;
            for (size_t i = 0; i < flat.size(); i++)
                if (flat[i].name == rec.second->name) {
                    existPos = i;
                    break;
                }
            if (existPos == -1)
                flat.push_back(*rec.second);
            else if (pstd::find(parents.begin(), parents.end(), rec.second->name) == parents.end())
                flat[existPos].time += rec.second->time;

            parents.push_back(rec.second->name);
            me(me, rec.second->children, parents);
        }
    };
    Flatten(Flatten, profilerRecord->children, {});
    pstd::sort(flat, [](const Record& lhs, const Record& rhs) { return lhs.time > rhs.time; });

    size_t maxNameLength = 0;
    double totalTime = 0.0;
    for (auto& rec : profilerRecord->children)
        totalTime += rec.second->time;
    for (auto& rec : flat)
        if (rec.time / totalTime > 0.005f)
            maxNameLength = pstd::max(maxNameLength, rec.name.size());
    for (const auto& rec : flat)
        if (rec.time / totalTime > 0.005f)
            LOG("| &<: &9 calls &9.2 ms(avg) &10.1 ms(total) &3.2 %", Format(maxNameLength),
                rec.name.c_str(), rec.sampleCount, rec.time / (double)rec.sampleCount, rec.time,
                100.0 * rec.time / totalTime);

    LOG("\n");
}
Profiler::Profiler(pstd::string description) {
    pstd::shared_ptr<Record>& rec = profilerRecord->children[description];
    if (rec == nullptr)
        rec = pstd::make_shared<Record>();

    rec->name = description;
    rec->parent = profilerRecord;
    profilerRecord = rec;
}
Profiler::~Profiler() {
    pstd::shared_ptr<Record> rec = profilerRecord;

    rec->time += timer.ElapsedMs();
    rec->sampleCount++;

    profilerRecord = rec->parent;
}

static thread_local uint64_t profilePhase = {};
static pstd::map<uint64_t, uint64_t> profilePhaseRecords = {};
static uint64_t profileTotalSamples = 0;
static_assert(
    (int)ProfilePhase::NumPhase == sizeof(profilePhaseName) / sizeof(profilePhaseName[0]),
    "Expect ProfilePhase::NumPhase == sizeof(profilePhaseName) / sizeof(profilePhaseName[0])");

void RecordSampleCallback(int, siginfo_t*, void*) {
    profilePhaseRecords[profilePhase]++;
    profileTotalSamples++;
}
void SampledProfiler::Initialize() {
    struct sigaction sa;
    pstd::memset(&sa, 0, sizeof(sa));
    sa.sa_sigaction = RecordSampleCallback;
    sa.sa_flags = SA_RESTART | SA_SIGINFO;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGPROF, &sa, NULL);

    static struct itimerval timer;
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = 1000000 / 100;
    timer.it_value = timer.it_interval;

    CHECK_EQ(setitimer(ITIMER_PROF, &timer, NULL), 0);
}
void SampledProfiler::Finalize() {
    static struct itimerval timer;
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = 0;
    timer.it_value = timer.it_interval;
    CHECK_EQ(setitimer(ITIMER_PROF, &timer, NULL), 0);
    LOG("[SampledProfiler]Results:");

    pstd::vector<pstd::pair<pstd::string, uint64_t>> phaseStat((int)ProfilePhase::NumPhase);
    for (int i = 0; i < (int)ProfilePhase::NumPhase; i++)
        phaseStat[i].first = profilePhaseName[i];

    for (auto record : profilePhaseRecords) {
        for (int b = 0; b < (int)ProfilePhase::NumPhase; b++) {
            if (uint64_t(1) & (record.first >> b)) {
                phaseStat[b].second += record.second;
            }
        }
    }

    pstd::sort(phaseStat,
               [](pstd::pair<pstd::string, uint64_t> lhs, pstd::pair<pstd::string, uint64_t> rhs) {
                   return lhs.second > rhs.second;
               });

    size_t maxWidth = 0;
    for (auto pair : phaseStat) {
        maxWidth = pstd::max(pair.first.size(), maxWidth);
    }
    for (auto pair : phaseStat) {
        if (pair.second)
            LOG("| &<: &10 &4.2%", Format(maxWidth), pair.first, pair.second,
                100.0 * pair.second / profileTotalSamples);
    }

    LOG("");
}
SampledProfiler::SampledProfiler(ProfilePhase p) : p(p) {
    profilePhase |= uint64_t(1) << (int)p;
}
SampledProfiler::~SampledProfiler() {
    profilePhase &= ~(uint64_t(1) << (int)p);
}

}  // namespace pine