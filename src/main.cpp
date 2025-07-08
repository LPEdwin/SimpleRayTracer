#define FMT_HEADER_ONLY
#include "fmt/core.h"
#include "fmt/chrono.h"
#include "fmt/format.h"

#include <iostream>
#include <chrono>
#include <exception>

#include "core/camera.h"
#include "core/renderer.h"
#include "io/image.h"
#include "scenes/scene.h"
#include "scenes/cornell_box.h"

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
        .environmentMap = scene.environmentMap};
    renderer.Render(image, *scene.camera, *scene.objects);

    auto end = steady_clock::now();
    auto duration = duration_cast<seconds>(end - start);
    fmt::println("Elapsed {:%H:%M:%S}", duration);

    try
    {
        auto filename = fmt::format("output_{:%H.%M.%S}.bmp", duration);
        SaveBmp_sRGB(image, filename);
        fmt::println("BMP saved to {}", filename);
    }
    catch (const std::exception &e)
    {
        cerr << "Error writing BMP: " << e.what() << "\n";
    }

    return 0;
}
