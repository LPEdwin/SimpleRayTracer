#pragma once

#include "hittable.h"
#include "camera.h"
#include "render.h"

struct Scene
{
    std::shared_ptr<Hittable> objects;
    std::shared_ptr<Camera> camera;
    BackgroundFunc backgroundFunc = BlackBackground;
};