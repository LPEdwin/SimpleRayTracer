#define FMT_HEADER_ONLY
#include "fmt/core.h"
#include "fmt/chrono.h"
#include "fmt/format.h"

#ifdef _MSC_VER
  #define TINYEXR_USE_MINIZ 1
#else
  #define TINYEXR_USE_MINIZ 0
  #include <zlib.h>
#endif
#define TINYEXR_IMPLEMENTATION
#include "tinyexr/tinyexr.h"

#include <iostream>
#include <chrono>
#include <exception>

#include "core/camera.h"
#include "core/renderer.h"
#include "io/image.h"
#include "scenes/scene.h"
#include "scenes/cornell_box.h"
#include "io/exr_save.h"

using namespace std;
using namespace std::chrono;

int main()
{
    fmt::println("Building Scene...");
    auto scene = CornellBox();
    auto height = 720;
    auto width = static_cast<int>(height * scene.camera->AspectRatio());
    fmt::println("Image size: {} x {}", width, height);
    Image image(width, height);

    auto start = steady_clock::now();
    Renderer renderer{
        .maxDepth = 50,
        .samplesPerPixel = 100,
        .maxThreadCount = 0,
        .environmentMap = scene.environmentMap,
        .mixtureSamplingWeight = 0.5};
    renderer.Render(image, *scene.camera, *scene.objects, *scene.lights);

    auto end = steady_clock::now();
    auto duration = duration_cast<seconds>(end - start);
    fmt::println("Elapsed {:%H:%M:%S}", duration);

    try
    {
        auto filename = fmt::format("output_{:%H.%M.%S}.bmp", duration);
        SaveBmp_sRGB(image, filename);
        fmt::println("BMP saved to {}", filename);
    }
    catch (const std::exception& e)
    {
        fmt::println(stderr, "Error writing BMP: {}", e.what());
    }

    try
    {
        auto exrfilename = fmt::format("output_{:%H.%M.%S}.exr", duration);
        SaveEXRImage(image, exrfilename);
        fmt::println("EXR saved to {}", exrfilename);
    }
    catch (const std::exception& e)
    {
        fmt::println(stderr, "Error writing EXR: {}", e.what());
    }

    return 0;
}
