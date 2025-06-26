#pragma once

#include "hittable.h"
#include "hittable_list.h"
#include "material.h"
#include "sphere.h"
#include "camera.h"

struct Scene
{
    shared_ptr<Hittable> objects;
    shared_ptr<Camera> camera;
};

Scene CreateFinalScene()
{
    HittableList scene_objects{};

    auto ground_material = make_shared<Lambertian>(Color(0.5, 0.5, 0.5));
    scene_objects.add(make_shared<Sphere>(Point3(0, -1000, 0), 1000, ground_material));

    for (int a = -11; a < 11; a++)
    {
        for (int b = -11; b < 11; b++)
        {
            auto choose_mat = RandomDouble();
            Point3 center(a + 0.9 * RandomDouble(), 0.2, b + 0.9 * RandomDouble());

            if ((center - Point3(4, 0.2, 0)).Length() > 0.9)
            {
                shared_ptr<Material> sphere_material;

                if (choose_mat < 0.8)
                {
                    // diffuse
                    auto albedo = Color::Random() * Color::Random();
                    sphere_material = make_shared<Lambertian>(albedo);
                    scene_objects.add(make_shared<Sphere>(center, 0.2, sphere_material));
                }
                else if (choose_mat < 0.95)
                {
                    // metal
                    auto albedo = Color::Random(0.5, 1);
                    auto fuzz = RandomDouble(0, 0.5);
                    sphere_material = make_shared<Metal>(albedo, fuzz);
                    scene_objects.add(make_shared<Sphere>(center, 0.2, sphere_material));
                }
                else
                {
                    // glass
                    sphere_material = make_shared<Dielectric>(1.5);
                    scene_objects.add(make_shared<Sphere>(center, 0.2, sphere_material));
                }
            }
        }
    }

    auto material1 = make_shared<Dielectric>(1.5);
    scene_objects.add(make_shared<Sphere>(Point3(0, 1, 0), 1.0, material1));

    auto material2 = make_shared<Lambertian>(Color(0.4, 0.2, 0.1));
    scene_objects.add(make_shared<Sphere>(Point3(-4, 1, 0), 1.0, material2));

    auto material3 = make_shared<Metal>(Color(0.7, 0.6, 0.5), 0.0);
    scene_objects.add(make_shared<Sphere>(Point3(4, 1, 0), 1.0, material3));

    auto camera = make_shared<Camera>(Vector3(13, 2, 3), Vector3(0, 0, 0), 20.0, 16.0 / 9.0, 10.0, 0.1);

    return Scene{
        .objects = make_shared<HittableList>(scene_objects),
        .camera = camera};
}
