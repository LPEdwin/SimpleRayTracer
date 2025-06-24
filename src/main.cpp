#include <iostream>
#include <cmath>
#include <fstream>
#include <cstdint>
#include <vector>
#include <numbers>

#include "vector3.h"
#include "image.h"
#include "camera.h"
#include "sphere.h"
#include "render.h"
#include "hittable_list.h"

using namespace std;
using namespace std::numbers;

int main()
{
    Camera camera(Vector3(0, 0, -5), Vector3(0, 0, 1), 60.0f, 16.0 / 9.0);
    // Sphere s1(Vector3(-2.5, 2.0f, 3), 1.0f);
    // Sphere s2(Vector3(0, 0, 0), 1.0f);
    // vector<Sphere> spheres = {s1, s2};

    HittableList world{};
    world.add(make_shared<Sphere>(Point3(0, 0, -1), 0.5));
    world.add(make_shared<Sphere>(Point3(0, -100.5, -1), 100));

    Image image(1280, 720);

    Render(camera, world, image);

    try
    {
        auto filename = "output.bmp";
        WriteImageToBMP(image, filename);
        std::cout << "BMP saved to " << filename << "\n";
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error writing BMP: " << e.what() << "\n";
    }

    return 0;
}
