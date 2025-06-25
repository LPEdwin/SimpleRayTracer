#pragma once

#include "vector3.h"
#include "ray.h"
#include "hittable.h"
#include "material.h"

struct Sphere : Hittable
{
    Vector3 center;
    double radius;
    std::shared_ptr<Material> material;

    Sphere() : center(Vector3()), radius(1.0), material(DefaultMaterial()) {}
    Sphere(Vector3 c, double r) : center(c), radius(r), material(DefaultMaterial()) {}
    Sphere(Vector3 c, double r, std::shared_ptr<Material> m) : center(c), radius(r), material(m)
    {
        if (!material)
            throw std::invalid_argument("Material must not be null");
    }

    std::optional<HitResult> Hit(const Ray &ray, double t_min, double t_max) const override
    {
        Vector3 oc = center - ray.origin;
        double a = ray.direction.LengthSquared();
        double b = Dot(oc, ray.direction);
        double c = oc.LengthSquared() - radius * radius;
        double discriminant = b * b - a * c;

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
        auto hit = HitResult(hit_point, outward_normal, root, material);
        hit.set_face_normal(ray, outward_normal);
        return hit;
    }
};