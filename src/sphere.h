#pragma once

#include "vector3.h"
#include "ray.h"
#include "hittable.h"

struct Sphere : Hittable
{
    Vector3 center;
    double radius;

    Sphere() : center(), radius(1.0f) {}
    Sphere(Vector3 c, float r) : center(c), radius(r) {}

    std::optional<HitResult> Hit(const Ray &ray, double t_min, double t_max) const override
    {
        Vector3 oc = center - ray.origin;
        float a = ray.direction.LengthSquared();
        float b = Dot(oc, ray.direction);
        float c = oc.LengthSquared() - radius * radius;
        float discriminant = b * b - a * c;

        if (discriminant < 0)
            return std::nullopt;

        auto sqrtd = std::sqrt(discriminant);

        auto root = (b - sqrtd) / a;
        if (root < t_min || t_max < root)
        {
            root = (b + sqrtd) / a;
            if (root < t_min || t_max < root)
                return std::nullopt;
        }

        Point3 hit_point = ray.At(root);
        // Vector3 outward_normal = (hit_point - center) / radius;
        Vector3 outward_normal = UnitVector(hit_point - center);
        auto hit = HitResult(hit_point, outward_normal, root);
        hit.set_face_normal(ray, outward_normal);
        return hit;
    }
};