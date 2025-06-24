#pragma once

#include "vector3.h"
#include "ray.h"

struct Sphere
{
    Vector3 center;
    float radius;

    Sphere() : center(), radius(1.0f) {}
    Sphere(Vector3 c, float r) : center(c), radius(r) {}

    bool Intersect(const Ray &ray, float &t) const
    {
        Vector3 oc = center - ray.origin;
        float a = ray.direction.LengthSquared();
        float b = Dot(oc, ray.direction);
        float c = oc.LengthSquared() - radius * radius;
        float discriminant = b * b - a * c;
        if (discriminant < 0)
        {
            return false;
        }
        else
        {
            t = (-b - sqrt(discriminant)) / a;
            return true;
        }
    }
};