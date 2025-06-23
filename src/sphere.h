#pragma once

#include "vec3.h"
#include "ray.h"

struct Sphere
{
    vec3 center;
    float radius;
    
    Sphere() : center(), radius(1.0f) {}
    Sphere(vec3 c, float r) : center(c), radius(r) {}

    bool Intersect(const Ray &ray, float &t) const
    {
        vec3 oc = ray.origin - center;
        float a = dot(ray.direction, ray.direction);
        float b = 2.0f * dot(oc, ray.direction);
        float c = dot(oc, oc) - radius * radius;
        float discriminant = b * b - 4 * a * c;
        if (discriminant < 0)
        {
            return false;
        }
        else
        {
            t = (-b - sqrt(discriminant)) / (2.0f * a);
            return true;
        }
    }
};