#pragma once

#include "scenes/scene.h"
#include "collision/bvh_node.h"
#include "quad.h"

Scene CreateQuadsScene()
{
    HittableList world;

    // Materials
    auto left_red = make_shared<Lambertian>(Color(1.0, 0.2, 0.2));
    auto back_green = make_shared<Lambertian>(Color(0.2, 1.0, 0.2));
    auto right_blue = make_shared<Lambertian>(Color(0.2, 0.2, 1.0));
    auto upper_orange = make_shared<Lambertian>(Color(1.0, 0.5, 0.0));
    auto lower_teal = make_shared<Lambertian>(Color(0.2, 0.8, 0.8));

    // Quads
    world.add(make_shared<Quad>(Point3(-3, -2, 5), Vector3(0, 0, -4), Vector3(0, 4, 0), left_red));
    world.add(make_shared<Quad>(Point3(-2, -2, 0), Vector3(4, 0, 0), Vector3(0, 4, 0), back_green));
    world.add(make_shared<Quad>(Point3(3, -2, 1), Vector3(0, 0, 4), Vector3(0, 4, 0), right_blue));
    world.add(make_shared<Quad>(Point3(-2, 3, 1), Vector3(4, 0, 0), Vector3(0, 0, 4), upper_orange));
    world.add(make_shared<Quad>(Point3(-2, -3, 5), Vector3(4, 0, 0), Vector3(0, 0, -4), lower_teal));

    Camera cam(Point3(0, 0, 9), Point3(0, 0, 0), 80.0, 1.0);
    return Scene{BvhNode::Build(world.shapes),
                 make_shared<Camera>(cam),
                 GradientBackground};
}