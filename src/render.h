#pragma once

#include <limits>

#include "camera.h"
#include "sphere.h"
#include "image.h"
#include "ray.h"
#include "vector3.h"
#include "hittable.h"

double inf = std::numeric_limits<double>::infinity();

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
                auto dir = UnitVector(ray.direction);

                if (auto hit = world.Hit(ray, 0.0, inf))
                {
                    color += 0.5 * (hit->normal + Color(1, 1, 1));
                }
                else
                {
                    auto a = 0.5 * (dir.y() + 1.0);
                    color += (1.0 - a) * Vector3(1.0, 1.0, 1.0) + a * Vector3(0.5, 0.7, 1.0);
                }
            }
            image.pixels[image.height - 1 - y][x] = color / samplesPerPixel;
        }
    }
};
