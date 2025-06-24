#pragma once

#include "vector3.h"

class Ray
{
public:
    Ray(const Point3 &origin, const Vector3 &direction, double time = 0.0) : origin(origin), direction(direction), time(time) {}

    const Point3 origin;
    const Vector3 direction;
    double const time;

    Point3 At(double t) const
    {
        return origin + t * direction;
    }
};