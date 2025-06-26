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
#include "scene.h"

using namespace std;
using namespace std::numbers;
using namespace std::chrono;

int main()
{
    auto scene = CreateFinalScene();
    auto height = 720;
    auto width = static_cast<int>(height * scene.camera->aspectRatio);
    cout << "Image size: " << width << "x" << height << "\n";
    Image image(width, height);

    auto start = steady_clock::now();
    Render(*scene.camera, *scene.objects, image);
    auto end = steady_clock::now();
    auto duration = duration_cast<seconds>(end - start);

    fmt::println("Elapsed {:%H:%M:%S}", duration);

    try
    {
        auto filename = "output.bmp";
        SaveBmp_sRGB(image, filename);
        cout << "BMP saved to " << filename << "\n";
        auto filename2 = fmt::format("output_{:%H.%M.%S}.bmp", duration);
        SaveBmp_sRGB(image, filename2);
        fmt::println("BMP saved to {}", filename2);
    }
    catch (const std::exception &e)
    {
        cerr << "Error writing BMP: " << e.what() << "\n";
    }

    return 0;
}
