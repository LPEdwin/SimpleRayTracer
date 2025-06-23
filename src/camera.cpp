#pragma once

#include "vec3.cpp"

struct Camera {
    vec3 position;
    vec3 direction;
    float fov;
    Camera() : position(), direction(0, 0, -1), fov(60.0f) {}
    Camera(vec3 pos, vec3 dir, float fov) : position(pos), direction(dir), fov(fov) {}
};
