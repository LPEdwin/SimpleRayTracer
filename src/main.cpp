
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

#include "vector3.h"
#include "image.h"
#include "camera.h"
#include "sphere.h"
#include "render.h"
#include "hittable_list.h"
#include "bvh_node.h"
#include "scenes/scene.h"
#include "scenes/quads_scene.h"
#include "scenes/final_01_scene.h"
#include "scenes/cornell_box.h"

using namespace std;
using namespace std::numbers;
using namespace std::chrono;

int main()
{
    auto scene = CreateCornellBox();
    auto height = 720;
    auto width = static_cast<int>(height * scene.camera->AspectRatio());
    fmt::print("Image size: {} x {}\n", width, height);
    Image image(width, height);

    auto start = steady_clock::now();

    Render(*scene.camera, *scene.objects, image, scene.backgroundFunc);

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
