#pragma once

#include "scenes/scene.h"
#include "io/object_loader.h"
#include "core/camera.h"
#include "collision/hittable_list.h"
#include "collision/instance.h"
#include "collision/bvh_node.h"

Scene MeshTest(string file, bool useBvh = false)
{
    auto triangles = LoadObjectFile(file);
    auto cam = std::make_shared<Camera>(Vector3(0, 278, -800), Vector3(0, 0, 0), 40.0, 1.0);
    auto hittables = AsHittables(triangles);
    shared_ptr<Hittable> mesh = std::make_shared<HittableList>(hittables);
    if (useBvh)
        mesh = BvhNode::Build(hittables);
    mesh = BvhNode::Build(hittables);
    auto scale = 250.0 / mesh->BoundingBox().LongestAxis().Length();
    auto scaled = std::make_shared<Instance>(mesh, Transform::FromScale(scale).RotateY(180));
    fmt::println("BB Scaled Mesh: {}", scaled->BoundingBox());
    fmt::println("Face Count: {}", hittables.size());
    return Scene{
        .objects = scaled,
        .camera = cam,
        .environmentMap = GradientMap::Sky(),
    };
}

Scene Pyramid()
{
    return MeshTest("assets/pyramid.obj");
}

Scene StanfordBunny()
{
    return MeshTest("assets/stanford-bunny.obj");
}