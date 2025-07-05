#pragma once

#include "core/hittable.h"
#include "core/camera.h"
#include "core/environment_map.h"

struct Scene
{
    std::shared_ptr<Hittable> objects;
    std::shared_ptr<Camera> camera;
    std::shared_ptr<EnvironmentMap> environmentMap = nullptr;
};