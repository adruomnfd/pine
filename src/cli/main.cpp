#include <core/integrator.h>
#include <core/scene.h>
#include <util/fileio.h>
#include <util/profiler.h>

#include <memory>

int main(int argc, char* argv[]) {
    using namespace pine;
    if (argc < 2) {
        LOG_FATAL("No input file");
        return 1;
    }

#ifndef NDEBUG
    LOG_WARNING("[Performance]Built in debug mode");
#endif

    {
        Profiler _("Main");
        SampledProfiler::Initialize();

        SampledSpectrum::Init();
        std::shared_ptr<Scene> scene = std::make_shared<Scene>();

        Parameters params = LoadScene(argv[1], scene.get());

        if (argc > 2)
            scene->parameters.Set("outputFileName", argv[2]);

        scene->Initialize();
        scene->integrator->Render();
        scene->Cleanup();
    }

    SampledProfiler::ReportStat();
    Profiler::ReportStat();

    return 0;
}