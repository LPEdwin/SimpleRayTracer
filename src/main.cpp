
#define FMT_HEADER_ONLY
#include "fmt/core.h"
#include "fmt/chrono.h"
#include "fmt/format.h"
#include "fmt/printf.h"

#include <iostream>
#include <cmath>
#include <fstream>
#include <cstdint>
#include <vector>
#include <numbers>
#include <chrono>

#include "core/vector3.h"
#include "io/image.h"
#include "core/camera.h"
#include "collision/sphere.h"
#include "core/renderer.h"
#include "collision/hittable_list.h"
#include "collision/bvh_node.h"
#include "scenes/scene.h"
#include "scenes/quads_scene.h"
#include "scenes/final_01_scene.h"
#include "scenes/cornell_box.h"
#include "scenes/final_02_scene.h"
#include "scenes/triangle_test.h"

using namespace std;
using namespace std::numbers;
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
