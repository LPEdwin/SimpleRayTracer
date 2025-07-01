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

    Sphere() : Sphere(Vector3(), 1.0) {}
    Sphere(Vector3 c, double r) : Sphere(c, r, DefaultMaterial()) {}
    Sphere(Vector3 c, double r, std::shared_ptr<Material> m) : center(c), radius(r), material(m)
    {
        if (!material)
            throw std::invalid_argument("Material must not be null");

        Vector3 rvec(r, r, r);
        bbox = AABB(c - rvec, c + rvec);
    }

private:
    AABB bbox;

public:
    AABB BoundingBox() const override
    {
        return bbox;
    }

    bool Hit(const Ray &ray, HitResult &hitResult, double t_min, double t_max) const override
    {
        Vector3 oc = center - ray.origin;
        double a = ray.direction.LengthSquared();
        double b = Dot(oc, ray.direction);
        double c = oc.LengthSquared() - radius * radius;
        double discriminant = b * b - a * c;

        if (discriminant < 0)
            return false;

        auto sqrtd = std::sqrt(discriminant);

        auto root = (b - sqrtd) / a;
        if (root < t_min || t_max < root)
        {
            root = (b + sqrtd) / a;
            if (root < t_min || t_max < root)
                return false;
        }

        Point3 hit_point = ray.At(root);
        // Vector3 outward_normal = (hit_point - center) / radius;
        Vector3 outward_normal = UnitVector(hit_point - center);
        hitResult.point = hit_point;
        hitResult.normal = outward_normal;
        hitResult.t = root;
        hitResult.material = material;
        hitResult.set_face_normal(ray, outward_normal);
        return true;
    }
};