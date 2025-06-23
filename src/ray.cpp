#pragma once

#include "vec3.cpp"

struct Ray {
    vec3 origin;
    vec3 direction;
    Ray(vec3 o, vec3 d) : origin(o), direction(d) {}
};