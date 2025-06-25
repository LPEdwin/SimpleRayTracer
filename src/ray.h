#pragma once

#include "vector3.h"

class Ray
{
public:
    Ray() : origin(Point3(0, 0, 0)), direction(Vector3(0, 0, 0)), time(0.0) {}
    Ray(const Point3 &origin, const Vector3 &direction, double time = 0.0) : origin(origin), direction(direction), time(time) {}

    Point3 origin;
    Vector3 direction;
    double time;

    Point3 At(double t) const
    {
        return origin + t * direction;
    }
};