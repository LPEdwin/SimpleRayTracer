#pragma once

#include "scenes/scene.h"
#include "io/object_loader.h"
#include "core/camera.h"
#include "collision/hittable_list.h"
#include "collision/instance.h"
#include "collision/bvh_node.h"
#include "core/transform.h"
#include "collision/quad.h"
#include "collision/experimental/flat_bvh.h"
#include "collision/experimental/static_bvh.h"

vector<shared_ptr<Hittable>> EmptyCornellBox()
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

    return world;
}

Scene MeshTest(string file)
{
    auto world = EmptyCornellBox();
    auto mesh = LoadAsTriangleList(file);
    auto scale = 250.0 / mesh->BoundingBox().LongestAxis().Length();
    auto scaled = std::make_shared<Instance>(mesh, Transform::FromTranslate(0, 0, 300).Scale(scale).RotateY(180));
    world.push_back(scaled);

    fmt::println("Scaled Mesh BB: {:.3f}", scaled->BoundingBox());

    auto cam = std::make_shared<Camera>(Vector3(0, 278, -800), Vector3(0, 278, 0), 40.0, 1.0);

    return Scene{
        .objects = BvhNode::Build(world),
        .camera = cam,
    };
}

Scene FlatMeshTest(string file)
{
    auto world = EmptyCornellBox();
    auto mesh = FlatBvh::Mesh::Create(file);
    fmt::println("Face Count: {}", mesh->FaceCount());
    fmt::println("BVH Node Count: {}", mesh->BvhNodeCount());
    auto scale = 250.0 / mesh->BoundingBox().LongestAxis().Length();
    auto scaled = std::make_shared<Instance>(mesh, Transform::FromTranslate(0, 0, 300).Scale(scale).RotateY(180));
    world.push_back(scaled);

    fmt::println("Scaled Mesh BB: {:.3f}", scaled->BoundingBox());

    auto cam = std::make_shared<Camera>(Vector3(0, 278, -800), Vector3(0, 278, 0), 40.0, 1.0);

    return Scene{
        .objects = BvhNode::Build(world),
        .camera = cam,
    };
}

Scene StaticMeshTest(string file)
{
    auto world = EmptyCornellBox();
    auto mesh = StaticBvh::Mesh::Create("assets/stanford-bunny.obj");
    fmt::println("Face Count: {}", mesh->FaceCount());
    auto scale = 250.0 / mesh->BoundingBox().LongestAxis().Length();
    auto scaled = std::make_shared<Instance>(mesh, Transform::FromTranslate(0, 0, 300).Scale(scale).RotateY(180));
    world.push_back(scaled);

    fmt::println("Scaled Mesh BB: {:.3f}", scaled->BoundingBox());

    auto cam = std::make_shared<Camera>(Vector3(0, 278, -800), Vector3(0, 278, 0), 40.0, 1.0);

    return Scene{
        .objects = BvhNode::Build(world),
        .camera = cam,
    };
}

Scene TriangleListTest(string file, bool useBvh = true)
{
    auto world = EmptyCornellBox();
    auto faces = LoadAsTriangleList(file);
    fmt::println("Face Count: {}", faces->shapes.size());
    shared_ptr<Hittable> mesh = faces;
    if (useBvh)
        mesh = BvhNode::Build(faces->shapes);
    auto scale = 250.0 / mesh->BoundingBox().LongestAxis().Length();
    mesh = std::make_shared<Instance>(mesh, Transform::FromTranslate(0, 0, 300).Scale(scale).RotateY(180));
    world.push_back(mesh);

    fmt::println("Scaled Mesh BB: {:.3f}", mesh->BoundingBox());

    auto cam = std::make_shared<Camera>(Vector3(0, 278, -800), Vector3(0, 278, 0), 40.0, 1.0);

    return Scene{
        .objects = BvhNode::Build(world),
        .camera = cam,
    };
}

Scene Pyramid(bool useBvh = true)
{
    return TriangleListTest("assets/pyramid.obj", useBvh);
}

Scene StanfordBunny(bool useBvh = true)
{
    return TriangleListTest("assets/stanford-bunny.obj", useBvh);
}

Scene StanfordBunnyAsStaticMesh()
{
    return StaticMeshTest("assets/stanford-bunny.obj");
}

Scene StanfordBunnyAsFlatMesh()
{
    return FlatMeshTest("assets/stanford-bunny.obj");
}