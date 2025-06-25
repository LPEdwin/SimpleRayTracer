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

    if (auto hit = world.Hit(ray, 0.001, inf))
    {
        auto r2 = RandomOnHemisphere(hit->normal);
        return 0.5 * GetColor(Ray(hit->point, r2, ray.time), world, depth - 1);
    }

    auto dir = UnitVector(ray.direction);
    auto a = 0.5 * (dir.y() + 1.0);
    return (1.0 - a) * Vector3(1.0, 1.0, 1.0) + a * Vector3(0.5, 0.7, 1.0);
}

void Render(const Camera &camera, const Hittable &world, Image &image, int samplesPerPixel = 10)
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
            image.pixels[image.height - 1 - y][x] = color / samplesPerPixel;
        }
    }
}