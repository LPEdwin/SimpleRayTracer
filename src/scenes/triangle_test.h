#pragma once

#include "collision/hittable.h"
#include "quad.h"
#include "camera.h"
#include "material.h"
#include "scenes/scene.h"
#include "bvh_node.h"
#include "vector3.h"
#include <box.h>
#include <transform.h>
#include <instance.h>
#include "collision/triangle.h"

Scene TriangleTest()
{
    vector<shared_ptr<Hittable>> world;

    auto red = make_shared<Lambertian>(Color(.65, .05, .05));
    auto white = make_shared<Lambertian>(Color(.73, .73, .73));
    auto green = make_shared<Lambertian>(Color(.12, .45, .15));
    auto light = make_shared<Emissive>(Color(15, 15, 15));
    auto metal = make_shared<Metal>(Color(0.8, 0.85, 0.88), 0.0);
    auto glass = make_shared<Dielectric>(1.5);

    double r(100.0);
    Point3 c(r, r, r);

    auto t1 = make_shared<Triangle>(Point3(-r, -r, 0), Point3(r, -r, 0), Point3(0, r, 0), red);
    world.push_back(t1);

    Camera cam(Vector3(0, 300, -800), Vector3(0, 0, 0), 40.0, 1.0);
    return Scene{BvhNode::Build(world), make_shared<Camera>(cam), GradientBackground};
}