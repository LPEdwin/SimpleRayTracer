#pragma once

#include "hittable.h"
#include "quad.h"
#include "camera.h"
#include "material.h"
#include "scenes/scene.h"
#include "bvh_node.h"
#include "vector3.h"

Scene CreateCornellBox()
{
    vector<shared_ptr<Hittable>> world;

    auto red = make_shared<Lambertian>(Color(.65, .05, .05));
    auto white = make_shared<Lambertian>(Color(.73, .73, .73));
    auto green = make_shared<Lambertian>(Color(.12, .45, .15));
    auto light = make_shared<Emissive>(Color(15, 15, 15));

    world.push_back(make_shared<Quad>(Point3(555, 0, 0), Vector3(0, 555, 0), Vector3(0, 0, 555), green));
    world.push_back(make_shared<Quad>(Point3(0, 0, 0), Vector3(0, 555, 0), Vector3(0, 0, 555), red));
    world.push_back(make_shared<Quad>(Point3(343, 554, 332), Vector3(-130, 0, 0), Vector3(0, 0, -105), light));
    world.push_back(make_shared<Quad>(Point3(0, 0, 0), Vector3(555, 0, 0), Vector3(0, 0, 555), white));
    world.push_back(make_shared<Quad>(Point3(555, 555, 555), Vector3(-555, 0, 0), Vector3(0, 0, -555), white));
    world.push_back(make_shared<Quad>(Point3(0, 0, 555), Vector3(555, 0, 0), Vector3(0, 555, 0), white));

    Camera cam(Vector3(278, 278, -800), Vector3(278, 278, 0), 40.0, 1.0);
    return Scene{BvhNode::Build(world), make_shared<Camera>(cam)};
}