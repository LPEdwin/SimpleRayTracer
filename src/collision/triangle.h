#pragma once

#include "vector3.h"
#include "ray.h"
#include "collision/hittable.h"
#include "collision/aabb.h"
#include "material.h"

struct Triangle final : public Hittable
{
    Point3 v0, v1, v2;
    Vector3 normal;
    std::shared_ptr<Material> material;

    Triangle(const Point3 &v0, const Point3 &v1, const Point3 &v2, std::shared_ptr<Material> material = DefaultMaterial())
        : v0(v0), v1(v1), v2(v2), material(material)
    {
        normal = UnitVector(Cross(v1 - v0, v2 - v0));
        Vector3 minPoint = Vector3::Min(v0, Vector3::Min(v1, v2));
        Vector3 maxPoint = Vector3::Max(v0, Vector3::Max(v1, v2));
        bbox = AABB(minPoint, maxPoint);
    }

    bool Hit(const Ray &ray, HitResult &hit, double t_min, double t_max) const override
    {
        constexpr double EPS = 1e-8;

        const Vector3 edge1 = v1 - v0;
        const Vector3 edge2 = v2 - v0;

        const Vector3 h = Cross(ray.direction, edge2);
        const double a = Dot(edge1, h);

        if (std::abs(a) < EPS)
            return false; // Ray is parallel to triangle

        const double f = 1.0 / a;
        const Vector3 s = ray.origin - v0;
        const double u = f * Dot(s, h);

        if (u < 0.0 || u > 1.0)
            return false;

        const Vector3 q = Cross(s, edge1);
        const double v = f * Dot(ray.direction, q);

        if (v < 0.0 || (u + v) > 1.0)
            return false;

        const double t = f * Dot(edge2, q);
        if (t < t_min || t > t_max)
            return false;

        // Ray hits triangle
        hit.t = t;
        hit.point = ray.At(t);

        // Compute geometric normal at hit time
        const Vector3 geometric_normal = UnitVector(Cross(edge1, edge2));
        hit.SetFaceNormal(ray, geometric_normal);
        hit.material = material;

        return true;
    }

    AABB BoundingBox() const override
    {
        return bbox;
    }

private:
    Triangle() = default;
    AABB bbox;
};