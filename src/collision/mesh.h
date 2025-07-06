#pragma once

#include "core/vector3.h"

struct Face
{
    Point3 v0, v1, v2;

    Face(Point3 v0, Point3 v1, Point3 v2)
        : v0(v0), v1(v1), v2(v2) {}
};