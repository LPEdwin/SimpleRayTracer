#include <iostream>
#include <cmath>
#include <fstream>
#include <cstdint>
#include <vector>
#include <numbers>

#include "vector3.h"
#include "image.h"
#include "camera.h"
#include <chrono>

#include "sphere.h"
#include "render.h"
#include "hittable_list.h"
#include "scene.h"
#include "timer.h"

using namespace std;
using namespace std::numbers;

int main()
{
    auto scene = CreateFinalScene();
    Image image(1280, 720);

    auto start = std::chrono::high_resolution_clock::now();
    Render(*scene.camera, *scene.objects, image);
    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Elapsed: " << formatDuration(end - start) << std::endl;

    try
    {
        auto filename = "output.bmp";
        SaveBmp_sRGB(image, filename);
        std::cout << "BMP saved to " << filename << "\n";
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error writing BMP: " << e.what() << "\n";
    }

    return 0;
}
