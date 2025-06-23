#include <iostream>
#include <cmath>
#include <fstream>
#include <cstdint>
#include <vector>
#include <numbers>

#include "vec3.h"
#include "image.h"
#include "camera.h"
#include "sphere.h"

using namespace std;
using namespace std::numbers;

void Render(const Camera &camera, const Sphere &sphere, Image &image)
{
    const vec3 pixelDelta = vec3(1.0f / image.width, 1.0f / image.height, 0.0f);
    for (int y = 0; y < image.height; ++y)
    {
        for (int x = 0; x < image.width; ++x)
        {
            Ray ray = camera.GetRay(x * pixelDelta.x(), pixelDelta.y() * (image.height - 1 - y));
            auto dir = unit_vector(ray.direction);
            float t = 0.0f;
            if (sphere.Intersect(Ray(camera.position, dir), t))
            {
                image.pixels[y][x] = vec3(1.0f, 0.0f, 0.0f);
            }
            else
            {
                auto a = 0.5 * (dir.y() + 1.0);
                image.pixels[y][x] = (1.0 - a) * vec3(1.0, 1.0, 1.0) + a * vec3(0.5, 0.7, 1.0);
            }
        }
    }
};

int main()
{
    Camera camera(vec3(0, 0, 5), vec3(0, 0, -1), 60.0f, 16.0 / 9.0);
    Sphere sphere(vec3(0, 0, 0), 1.0f);
    Image image(1280, 720);

    Render(camera, sphere, image);

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
