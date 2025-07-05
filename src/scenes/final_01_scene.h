#pragma once

#include <random>

#include "scenes/scene.h"
#include "collision/hittable.h"
#include "hittable_list.h"
#include "material.h"
#include "sphere.h"
#include "camera.h"
#include "collision/bvh_node.h"

Scene CreateFinalScene()
{
    vector<shared_ptr<Hittable>> scene_objects{};

    auto ground_material = make_shared<Lambertian>(Color(0.5, 0.5, 0.5));
    scene_objects.push_back(make_shared<Sphere>(Point3(0, -1000, 0), 1000, ground_material));

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
                    scene_objects.push_back(make_shared<Sphere>(center, 0.2, sphere_material));
                }
                else if (choose_mat < 0.95)
                {
                    // metal
                    auto albedo = Color::Random(0.5, 1);
                    auto fuzz = RandomDouble(0, 0.5);
                    sphere_material = make_shared<Metal>(albedo, fuzz);
                    scene_objects.push_back(make_shared<Sphere>(center, 0.2, sphere_material));
                }
                else
                {
                    // glass
                    sphere_material = make_shared<Dielectric>(1.5);
                    scene_objects.push_back(make_shared<Sphere>(center, 0.2, sphere_material));
                }
            }
        }
    }

    auto material1 = make_shared<Dielectric>(1.5);
    scene_objects.push_back(make_shared<Sphere>(Point3(0, 1, 0), 1.0, material1));

    auto material2 = make_shared<Lambertian>(Color(0.4, 0.2, 0.1));
    scene_objects.push_back(make_shared<Sphere>(Point3(-4, 1, 0), 1.0, material2));

    auto material3 = make_shared<Metal>(Color(0.7, 0.6, 0.5), 0.0);
    scene_objects.push_back(make_shared<Sphere>(Point3(4, 1, 0), 1.0, material3));

    auto camera = make_shared<Camera>(Vector3(13, 2, 3), Vector3(0, 0, 0), 20.0, 16.0 / 9.0, 10.0, 0.1);

    return Scene{
        .objects = BvhNode::Build(scene_objects, 0, scene_objects.size()),
        .camera = camera,
        .backgroundFunc = GradientBackground};
}

// Create a benchmark scene with precomputed random values for reproducibility
Scene CreateBenchmark01()
{
    vector<shared_ptr<Hittable>> scene_objects{};

    auto ground_material = make_shared<Lambertian>(Color(0.5, 0.5, 0.5));
    scene_objects.push_back(make_shared<Sphere>(Point3(0, -1000, 0), 1000, ground_material));

    // Precomputed random values with fixed seed
    std::mt19937 rng(42);
    std::uniform_real_distribution<double> dist(0.0, 1.0);

    // Precompute all random values needed for the scene
    struct PrecomputedRandom
    {
        double choose_mat;
        double center_x_offset;
        double center_z_offset;
        double albedo_r, albedo_g, albedo_b;
        double albedo2_r, albedo2_g, albedo2_b;
        double metal_fuzz;
    };

    std::vector<PrecomputedRandom> precomputed;
    for (int i = 0; i < 22 * 22; i++)
    { // 22x22 = 484 spheres
        PrecomputedRandom rnd;
        rnd.choose_mat = dist(rng);
        rnd.center_x_offset = dist(rng);
        rnd.center_z_offset = dist(rng);

        // Diffuse materials need 2 random colors (6 values)
        rnd.albedo_r = dist(rng);
        rnd.albedo_g = dist(rng);
        rnd.albedo_b = dist(rng);
        rnd.albedo2_r = dist(rng);
        rnd.albedo2_g = dist(rng);
        rnd.albedo2_b = dist(rng);

        // Metal material needs 1 random color + fuzz (4 values)
        rnd.metal_fuzz = dist(rng);

        precomputed.push_back(rnd);
    }

    int index = 0;
    for (int a = -11; a < 11; a++)
    {
        for (int b = -11; b < 11; b++)
        {
            const auto &rnd = precomputed[index++];

            Point3 center(
                a + 0.9 * rnd.center_x_offset,
                0.2,
                b + 0.9 * rnd.center_z_offset);

            if ((center - Point3(4, 0.2, 0)).Length() > 0.9)
            {
                shared_ptr<Material> sphere_material;

                if (rnd.choose_mat < 0.8)
                {
                    // Diffuse - use precomputed colors
                    Color albedo1(rnd.albedo_r, rnd.albedo_g, rnd.albedo_b);
                    Color albedo2(rnd.albedo2_r, rnd.albedo2_g, rnd.albedo2_b);
                    auto albedo = albedo1 * albedo2;
                    sphere_material = make_shared<Lambertian>(albedo);
                    scene_objects.push_back(make_shared<Sphere>(center, 0.2, sphere_material));
                }
                else if (rnd.choose_mat < 0.95)
                {
                    // Metal - use precomputed values
                    auto albedo = Color(rnd.albedo_r, rnd.albedo_g, rnd.albedo_b);
                    auto fuzz = rnd.metal_fuzz * 0.5; // Scale to [0, 0.5)
                    sphere_material = make_shared<Metal>(albedo, fuzz);
                    scene_objects.push_back(make_shared<Sphere>(center, 0.2, sphere_material));
                }
                else
                {
                    // Glass
                    sphere_material = make_shared<Dielectric>(1.5);
                    scene_objects.push_back(make_shared<Sphere>(center, 0.2, sphere_material));
                }
            }
        }
    }

    // Rest remains identical (deterministic)
    auto material1 = make_shared<Dielectric>(1.5);
    scene_objects.push_back(make_shared<Sphere>(Point3(0, 1, 0), 1.0, material1));

    auto material2 = make_shared<Lambertian>(Color(0.4, 0.2, 0.1));
    scene_objects.push_back(make_shared<Sphere>(Point3(-4, 1, 0), 1.0, material2));

    auto material3 = make_shared<Metal>(Color(0.7, 0.6, 0.5), 0.0);
    scene_objects.push_back(make_shared<Sphere>(Point3(4, 1, 0), 1.0, material3));

    auto camera = make_shared<Camera>(Vector3(13, 2, 3), Vector3(0, 0, 0), 20.0, 16.0 / 9.0, 10.0, 0.1);

    return Scene{
        .objects = BvhNode::Build(scene_objects, 0, scene_objects.size()),
        .camera = camera,
        .backgroundFunc = GradientBackground};
}