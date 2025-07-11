#pragma once

#include <cassert>
#include <memory>

#include "core/ray.h"
#include "core/vector3.h"
#include "core/aabb.h"

class Material; // Forward declaration

class HitResult
{
public:
    HitResult() = default;
    HitResult(const Point3 &point, const Vector3 &normal, double t, std::shared_ptr<Material> material)
        : point(point), normal(normal), t(t), material(material) {}

    Point3 point;
    // Always points in opposite direction of the incident ray.
    Vector3 normal;
    double t;
    // True if the ray is hitting the front face of the object.
    bool front_face;

    std::shared_ptr<Material> material;

    // outward_normal must be a unit vector.
    void SetFaceNormal(const Ray &ray, const Vector3 &outward_normal)
    {
        front_face = Dot(ray.direction, outward_normal) < 0;
        normal = front_face ? outward_normal : -outward_normal;
    }
};

class Hittable
{
public:
    virtual ~Hittable() = default;
    virtual bool Hit(const Ray &ray, HitResult &hitResult, double t_min, double t_max) const = 0;
    virtual AABB BoundingBox() const = 0;
};
