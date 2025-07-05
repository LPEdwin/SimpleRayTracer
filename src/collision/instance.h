#pragma once

#include "core/hittable.h"
#include "core/transform.h"
#include "core/aabb.h"
#include "core/ray.h"

class Instance : public Hittable
{
private:
    shared_ptr<Hittable> hittable;
    Transform transform;
    Transform inverse_transform;
    AABB bbox;

public:
    Instance(shared_ptr<Hittable> hittable, const Transform &transform)
        : hittable(hittable),
          transform(transform),
          inverse_transform(transform.Inverse()),
          bbox(AABB::Transformed(hittable->BoundingBox(), transform)) {}

    AABB BoundingBox() const override
    {
        return bbox;
    }

    bool Hit(const Ray &ray, HitResult &hit, double t_min, double t_max) const override
    {
        // Transform the ray to the object's local space
        Ray local_ray(inverse_transform * ray.origin,
                      inverse_transform.TransformDirection(ray.direction),
                      ray.time);

        // Call the underlying hittable's Hit method
        if (hittable->Hit(local_ray, hit, t_min, t_max))
        {
            // Transform the hit record back to world space
            hit.point = transform * hit.point;
            hit.normal = transform.TransformNormal(hit.normal);
            return true;
        }
        return false;
    }
};