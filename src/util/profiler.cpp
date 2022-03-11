#include <util/profiler.h>

#include <mutex>
#include <atomic>
#include <vector>
#include <algorithm>

#include <signal.h>
#include <sys/time.h>
#include <string.h>

namespace pine {

static std::shared_ptr<Profiler::Record> profilerRecord = std::make_shared<Profiler::Record>();

void Profiler::ReportStat() {
    if (verbose == false)
        return;
    LOG("[Profiler]==================Results==================");

    LOG("#structured:");
    auto ReportRecord = [](auto& me, Record record, size_t indent, double totalTime) -> void {
        if (totalTime != 0.0f && record.time / totalTime < 0.005f)
            return;
        if (record.name != "") {
            LOG("& &< &9 calls &10.1 ms    &3.2 %", Format(indent), "", Format(40 - indent),
                record.name.c_str(), record.sampleCount, record.time,
                (totalTime == 0.0f) ? 100.0 : 100.0 * record.time / totalTime);
            indent += 2;
        }

        std::vector<Record> sorted;
        for (auto& rec : record.children)
            sorted.push_back(*rec.second);
        std::sort(sorted.begin(), sorted.end(),
                  [](const Record& lhs, const Record& rhs) { return lhs.time > rhs.time; });
        for (auto& rec : sorted)
            me(me, rec, indent, record.time);
    };
    ReportRecord(ReportRecord, *profilerRecord, 0, 0.0f);

    LOG("\n#flattened:");
    std::vector<Record> flat;
    auto Flatten = [&](auto& me, const auto& map, std::vector<std::string> parents) -> void {
        for (auto& rec : map) {
            int existPos = -1;
            for (size_t i = 0; i < flat.size(); i++)
                if (flat[i].name == rec.second->name) {
                    existPos = i;
                    break;
                }
            if (existPos == -1)
                flat.push_back(*rec.second);
            else if (std::find(parents.begin(), parents.end(), rec.second->name) == parents.end())
                flat[existPos].time += rec.second->time;

            parents.push_back(rec.second->name);
            me(me, rec.second->children, parents);
        }
    };
    Flatten(Flatten, profilerRecord->children, {});
    std::sort(flat.begin(), flat.end(),
              [](const Record& lhs, const Record& rhs) { return lhs.time > rhs.time; });

    size_t maxNameLength = 0;
    double totalTime = 0.0;
    for (auto& rec : profilerRecord->children)
        totalTime += rec.second->time;
    for (auto& rec : flat)
        if (rec.time / totalTime > 0.005f)
            maxNameLength = std::max(maxNameLength, rec.name.size());
    for (const auto& rec : flat)
        if (rec.time / totalTime > 0.005f)
            LOG(" &<: &9 calls &9.2 ms(avg) &10.1 ms(total) &3.2 %", Format(maxNameLength),
                rec.name.c_str(), rec.sampleCount, rec.time / (double)rec.sampleCount, rec.time,
                100.0 * rec.time / totalTime);

    profilerRecord = std::make_shared<Record>();
    LOG("\n");
}
Profiler::Profiler(std::string description) {
    if (verbose == false)
        return;
    std::shared_ptr<Record>& rec = profilerRecord->children[description];
    if (rec == nullptr)
        rec = std::make_shared<Record>();

    rec->name = description;
    rec->parent = profilerRecord;
    profilerRecord = rec;
}
Profiler::~Profiler() {
    if (verbose == false)
        return;
    std::shared_ptr<Record> rec = profilerRecord;

    rec->time += timer.ElapsedMs();
    rec->sampleCount++;

    profilerRecord = rec->parent;
}

static thread_local uint64_t profilePhase = {};
static std::unordered_map<uint64_t, uint64_t> profilePhaseRecords = {};
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
    memset(&sa, 0, sizeof(sa));
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
void SampledProfiler::ReportStat() {
    std::vector<std::pair<std::string, uint64_t>> phaseStat((int)ProfilePhase::NumPhase);
    for (int i = 0; i < (int)ProfilePhase::NumPhase; i++)
        phaseStat[i].first = profilePhaseName[i];

    for (auto record : profilePhaseRecords) {
        for (int b = 0; b < (int)ProfilePhase::NumPhase; b++) {
            if (uint64_t(1) & (record.first >> b)) {
                phaseStat[b].second += record.second;
            }
        }
    }

    std::sort(phaseStat.begin(), phaseStat.end(),
              [](std::pair<std::string, uint64_t> lhs, std::pair<std::string, uint64_t> rhs) {
                  return lhs.second > rhs.second;
              });

    LOG("[SampledProfiler]==================Results==================");

    size_t maxWidth = 0;
    for (auto pair : phaseStat) {
        maxWidth = std::max(pair.first.size(), maxWidth);
    }
    for (auto pair : phaseStat) {
        if (pair.second)
            LOG(" &<: &10 &4.2%", Format(maxWidth), pair.first, pair.second,
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