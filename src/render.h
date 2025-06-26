#pragma once

#include <limits>

#include "camera.h"
#include "sphere.h"
#include "image.h"
#include "ray.h"
#include "vector3.h"
#include "hittable.h"

static const double inf = std::numeric_limits<double>::infinity();

Color GetColor(const Ray &ray, const Hittable &world, int depth = 50)
{
    if (depth <= 0)
        return Color(0, 0, 0);

    HitResult hit{};
    if (world.Hit(ray, hit, 0.001, inf))
    {
        Color attenuation;
        Ray r2;
        if (hit.material->Scatter(ray, hit, attenuation, r2))
            return attenuation * GetColor(r2, world, depth - 1);

        // If the ray does not scatter, return black.
        return Color(0, 0, 0);
    }

    auto dir = UnitVector(ray.direction);
    auto a = 0.5 * (dir.y() + 1.0);
    return (1.0 - a) * Vector3(1.0, 1.0, 1.0) + a * Vector3(0.5, 0.7, 1.0);
}

void updateProgress(int current, int total)
{
    static bool first_call = true;
    if (first_call)
    {
        std::cout << "\033[?25l"; // Hide cursor
        first_call = false;
    }

    int percent = static_cast<int>(100.0 * current / total);
    std::cout << "\rProgress: " << std::setw(3) << percent << "% completed" << std::flush;

    if (current == total)
    {
        std::cout << "\033[?25h" << std::endl; // Show cursor and newline
    }
}

void Render(const Camera &camera, const Hittable &world, Image &image, int samplesPerPixel = 100)
{
    const Vector3 pixelDelta = Vector3(1.0f / image.width, 1.0f / image.height, 0.0f);

    for (int y = 0; y < image.height; ++y)
    {
        for (int x = 0; x < image.width; ++x)
        {
            Color color(0, 0, 0);
            for (int s = 0; s < samplesPerPixel; ++s)
            {
                auto aaOffset = Vector3(RandomDouble() - 0.5, RandomDouble() - 0.5, 0.0);
                Ray ray = camera.GetRay((x + aaOffset.x()) * pixelDelta.x(),
                                        (y + aaOffset.y()) * pixelDelta.y());
                color += GetColor(ray, world);
            }
            image.pixels[y][x] = color / samplesPerPixel;
        }
        updateProgress(y + 1, image.height);
    }
}