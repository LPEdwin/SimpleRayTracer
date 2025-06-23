#pragma once

#include "vec3.h"

class Ray
{
public:
    Ray(const point3 &origin, const vec3 &direction, double time = 0.0) : origin(origin), direction(direction), time(time) {}

    const point3 origin;
    const vec3 direction;
    double const time;

    point3 at(double t) const
    {
        return origin + t * direction;
    }
};