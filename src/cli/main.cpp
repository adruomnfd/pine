#include <core/integrator.h>
#include <core/scene.h>
#include <util/fileio.h>
#include <util/profiler.h>

int main(int argc, char* argv[]) {
    using namespace pine;
    if (argc < 2) {
        LOG("pine [filename]");
        return 0;
    }

#ifndef NDEBUG
    LOG_WARNING("[Performance]Built in debug mode");
#endif

    {
        Profiler _("Main");
        SampledProfiler::Initialize();

        SampledSpectrum::Initialize();

        std::shared_ptr<Scene> scene = std::make_shared<Scene>();
        LoadScene(argv[1], scene.get());

        scene->integrator->Render();
    }

    SampledProfiler::ReportStat();
    Profiler::ReportStat();

    return 0;
}