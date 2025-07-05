#pragma once

#include <vector>
#include <memory>

#include "core/hittable.h"
#include "core/ray.h"

using std::make_shared;
using std::shared_ptr;
using std::vector;

class HittableList : public Hittable
{
public:
    vector<shared_ptr<Hittable>> shapes;

    HittableList() {}
    HittableList(shared_ptr<Hittable> shape) { add(shape); }

    void clear() { shapes.clear(); }

    void add(shared_ptr<Hittable> shape)
    {
        shapes.push_back(shape);
        bbox = AABB(bbox, shape->BoundingBox());
    }

    bool Hit(const Ray &ray, HitResult &hit, double t_min, double t_max) const override
    {
        auto closest_so_far = t_max;
        auto has_hit = false;
        HitResult temp_hit;
        for (const auto &s : shapes)
        {
            if (s->Hit(ray, temp_hit, t_min, closest_so_far))
            {
                closest_so_far = temp_hit.t;
                hit = temp_hit;
                has_hit = true;
            }
        }

        return has_hit;
    }

    AABB BoundingBox() const override { return bbox; }

private:
    AABB bbox = AABB();
};