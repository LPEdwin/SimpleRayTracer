#pragma once

#include "core/hittable.h"
#include "core/camera.h"
#include "collision/quad.h"
#include "core/environment_map.h"
#include "vector"

struct Scene
{
    std::shared_ptr<Hittable> objects;
    std::shared_ptr<Camera> camera;
    std::shared_ptr<std::vector<std::shared_ptr<Quad>>> lights = std::make_shared<std::vector<std::shared_ptr<Quad>>>();
    std::shared_ptr<EnvironmentMap> environmentMap = nullptr;
};