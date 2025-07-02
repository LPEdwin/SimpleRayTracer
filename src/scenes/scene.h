#pragma once

#include "hittable.h"
#include "camera.h"

struct Scene
{
    shared_ptr<Hittable> objects;
    shared_ptr<Camera> camera;
};