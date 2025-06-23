#pragma once

#include "vec3.cpp"

struct Ray
{
    vec3 origin;
    vec3 direction;
    double time;

    Ray(vec3 origin, vec3 direction, double time = 0.0) : origin(origin), direction(direction), time(time) {}
};