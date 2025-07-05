#pragma once

#include "hittable.h"
#include "quad.h"
#include "camera.h"
#include "material.h"
#include "scenes/scene.h"
#include "bvh_node.h"
#include "vector3.h"
#include <box.h>
#include <transform.h>
#include <instance.h>

Scene CreateCornellBox()
{
    vector<shared_ptr<Hittable>> world;

    auto red = make_shared<Lambertian>(Color(.65, .05, .05));
    auto white = make_shared<Lambertian>(Color(.73, .73, .73));
    auto green = make_shared<Lambertian>(Color(.12, .45, .15));
    auto light = make_shared<Emissive>(Color(15, 15, 15));

    world.push_back(make_shared<Quad>(Point3(277.5, 0, 0), Vector3(0, 555, 0), Vector3(0, 0, 555), green));
    world.push_back(make_shared<Quad>(Point3(-277.5, 0, 0), Vector3(0, 555, 0), Vector3(0, 0, 555), red));
    world.push_back(make_shared<Quad>(Point3(65.5, 554, 332), Vector3(-130, 0, 0), Vector3(0, 0, -105), light));
    world.push_back(make_shared<Quad>(Point3(-277.5, 0, 0), Vector3(555, 0, 0), Vector3(0, 0, 555), white));
    world.push_back(make_shared<Quad>(Point3(277.5, 555, 555), Vector3(-555, 0, 0), Vector3(0, 0, -555), white));
    world.push_back(make_shared<Quad>(Point3(-277.5, 0, 555), Vector3(555, 0, 0), Vector3(0, 555, 0), white));

    shared_ptr<Hittable> box1 = CreateBox(Point3(0, 0, 0), Point3(165, 330, 165), white);
    box1 = make_shared<Instance>(box1, Transform::FromTranslate(-12.5, 0, 295).RotateY(15));
    world.push_back(box1);

    shared_ptr<Hittable> box2 = CreateBox(Point3(0, 0, 0), Point3(165, 165, 165), white);
    box2 = make_shared<Instance>(box2, Transform::FromTranslate(-147.5, 0, 65).RotateY(-18));
    world.push_back(box2);

    Camera cam(Vector3(0, 278, -800), Vector3(0, 278, 0), 40.0, 1.0);
    return Scene{BvhNode::Build(world), make_shared<Camera>(cam)};
}