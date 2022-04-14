#include <core/integrator.h>
#include <core/scene.h>
#include <util/fileio.h>
#include <util/profiler.h>

int main(int argc, char* argv[]) {
    using namespace pine;
    if (argc != 2) {
        LOG("Usage: pine [filename]");
        return 0;
    }

#ifndef NDEBUG
    LOG_WARNING("[Performance]Built in debug mode");
#endif

    Profiler::Initialize();
    SampledProfiler::Initialize();
    SampledSpectrum::Initialize();

    auto scene = std::make_shared<Scene>();
    LoadScene(argv[1], scene.get());
    scene->integrator->Render();

    SampledProfiler::Finalize();
    Profiler::Finalize();

    return 0;
}