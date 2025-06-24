#pragma once

#include <vector>
#include <memory>

#include "hittable.h"
#include "ray.h"

using std::make_shared;
using std::shared_ptr;
using std::vector;

class HittableList : public Hittable
{
public:
    vector<shared_ptr<Hittable>> shapes;

    HittableList() {}
    HittableList(shared_ptr<Hittable> object) { add(object); }

    void clear() { shapes.clear(); }

    void add(shared_ptr<Hittable> object)
    {
        shapes.push_back(object);
    }

    optional<HitResult> Hit(const Ray &ray, double t_min, double t_max) const override
    {
        optional<HitResult> hit{};
        auto closest_so_far = t_max;

        for (const auto &s : shapes)
        {
            if (auto temp_hit = s->Hit(ray, t_min, closest_so_far))
            {
                closest_so_far = temp_hit->t;
                hit = *temp_hit;
            }
        }

        if (hit)
            return hit;
        return std::nullopt;
    }
};