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

using namespace std;
using namespace std::numbers;

int main()
{
    Camera camera(Vector3(0, 0, 0), Vector3(0, 0, -1), 90.0f, 16.0 / 9.0);

    HittableList world{};
    // world.add(make_shared<Sphere>(Point3(0, 0, -1), 0.5));
    // world.add(make_shared<Sphere>(Point3(0, -100.5, -1), 100));

    auto material_ground = make_shared<Lambertian>(Color(0.8, 0.8, 0.0));
    auto material_center = make_shared<Lambertian>(Color(0.1, 0.2, 0.5));
    auto material_left = make_shared<Metal>(Color(0.8, 0.8, 0.8), 0.3);
    auto material_right = make_shared<Metal>(Color(0.8, 0.6, 0.2), 1.0);

    world.add(make_shared<Sphere>(Point3(0.0, -100.5, -1.0), 100.0, material_ground));
    world.add(make_shared<Sphere>(Point3(0.0, 0.0, -1.2), 0.5, material_center));
    world.add(make_shared<Sphere>(Point3(-1.0, 0.0, -1.0), 0.5, material_left));
    world.add(make_shared<Sphere>(Point3(1.0, 0.0, -1.0), 0.5, material_right));

    Image image(1280, 720);

    auto start = std::chrono::high_resolution_clock::now();

    Render(camera, world, image);

    auto end = std::chrono::high_resolution_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << ms.count() << "ms" << std::endl;

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
