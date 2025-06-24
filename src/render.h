#include "camera.h"
#include "sphere.h"
#include "image.h"
#include "ray.h"
#include "vector3.h"

void Render(const Camera &camera, const vector<Sphere> &spheres, Image &image)
{
    const Vector3 pixelDelta = Vector3(1.0f / image.width, 1.0f / image.height, 0.0f);

    for (int y = 0; y < image.height; ++y)
    {
        for (int x = 0; x < image.width; ++x)
        {
            Ray ray = camera.GetRay(x * pixelDelta.x(), pixelDelta.y() * (image.height - 1 - y));
            auto dir = UnitVector(ray.direction);
            float t = 0.0f;
            auto hit = std::any_of(spheres.begin(), spheres.end(), [&](const Sphere &sphere)
                                   { return sphere.Intersect(ray, t); });
            if (hit)
            {
                image.pixels[y][x] = Vector3(1.0f, 0.0f, 0.0f);
            }
            else
            {
                auto a = 0.5 * (dir.y() + 1.0);
                image.pixels[y][x] = (1.0 - a) * Vector3(1.0, 1.0, 1.0) + a * Vector3(0.5, 0.7, 1.0);
            }
        }
    }
};

void Render(const Camera &camera, const Sphere &sphere, Image &image)
{
    Render(camera, vector<Sphere>{sphere}, image);
}
