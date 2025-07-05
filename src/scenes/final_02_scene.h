#pragma once

#include "collision/hittable.h"
#include "quad.h"
#include "camera.h"
#include "material.h"
#include "scenes/scene.h"
#include "collision/bvh_node.h"
#include "vector3.h"
#include "collision/box.h"
#include <transform.h>
#include <instance.h>

Scene CreateFinal02Scene()
{
    vector<shared_ptr<Hittable>> boxes1;
    auto ground = make_shared<Lambertian>(Color(0.48, 0.83, 0.53));

    int boxes_per_side = 20;
    for (int i = 0; i < boxes_per_side; i++)
    {
        for (int j = 0; j < boxes_per_side; j++)
        {
            auto w = 100.0;
            auto x0 = -1000.0 + i * w;
            auto z0 = -1000.0 + j * w;
            auto y0 = 0.0;
            auto x1 = x0 + w;
            auto y1 = RandomDouble(1, 101);
            auto z1 = z0 + w;

            boxes1.push_back(CreateBox(Point3(x0, y0, z0), Point3(x1, y1, z1), ground));
        }
    }

    vector<shared_ptr<Hittable>> world;

    world.push_back(BvhNode::Build(boxes1));

    auto light = make_shared<Emissive>(Color(7, 7, 7));
    world.push_back(make_shared<Quad>(Point3(123, 554, 147), Vector3(300, 0, 0), Vector3(0, 0, 265), light));
 
    world.push_back(make_shared<Sphere>(Point3(260, 150, 45), 50, make_shared<Dielectric>(1.5)));
    world.push_back(make_shared<Sphere>(Point3(0, 150, 145), 50, make_shared<Metal>(Color(0.8, 0.8, 0.9), 1.0)));

    auto boundary = make_shared<Sphere>(Point3(360, 150, 145), 70, make_shared<Dielectric>(1.5));
    world.push_back(boundary);

    vector<shared_ptr<Hittable>> boxes2;
    auto white = make_shared<Lambertian>(Color(.73, .73, .73));
    int ns = 1000;
    for (int j = 0; j < ns; j++)
    {
        boxes2.push_back(make_shared<Sphere>(Point3::Random(0, 165), 10, white));
    }

    world.push_back(make_shared<Instance>(BvhNode::Build(boxes2), Transform::FromRotateY(15).Translate(Vector3(-100, 270, 395))));

    Camera cam(Point3(478, 278, -600), Point3(278, 278, 0), 40, 1.0);

    return Scene{
        .objects = BvhNode::Build(world),
        .camera = make_shared<Camera>(cam),
    };
}