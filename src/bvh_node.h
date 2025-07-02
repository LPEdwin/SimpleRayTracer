#pragma once
//==============================================================================================
// Originally written in 2016 by Peter Shirley <ptrshrl@gmail.com>
//
// To the extent possible under law, the author(s) have dedicated all copyright and related and
// neighboring rights to this software to the public domain worldwide. This software is
// distributed without any warranty.
//
// You should have received a copy (see file COPYING.txt) of the CC0 Public Domain Dedication
// along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
//==============================================================================================

#include "aabb.h"
#include "hittable.h"
#include "hittable_list.h"

#include <algorithm>

// Bounding Volume Hierachy
class BvhNode : public Hittable
{
public:
    BvhNode(HittableList list) : BvhNode(list.shapes, 0, list.shapes.size())
    {
        // There's a C++ subtlety here. This constructor (without span indices) creates an
        // implicit copy of the hittable list, which we will modify. The lifetime of the copied
        // list only extends until this constructor exits. That's OK, because we only need to
        // persist the resulting bounding volume hierarchy.
    }

    BvhNode(std::vector<shared_ptr<Hittable>> &shapes, size_t start, size_t end)
    {
        // Build the bounding box of the span of source objects.
        bbox = AABB::empty;
        for (size_t i = start; i < end; i++)
            bbox = AABB(bbox, shapes[i]->BoundingBox());

        int axis = bbox.LongestAxis();

        auto comparator = (axis == 0)   ? BoxCompare_X
                          : (axis == 1) ? BoxCompare_Y
                                        : BoxCompare_Z;

        size_t object_span = end - start;

        if (object_span == 1)
        {
            left = right = shapes[start];
        }
        else if (object_span == 2)
        {
            left = shapes[start];
            right = shapes[start + 1];
        }
        else
        {
            std::sort(std::begin(shapes) + start, std::begin(shapes) + end, comparator);

            auto mid = start + object_span / 2;
            left = make_shared<BvhNode>(shapes, start, mid);
            right = make_shared<BvhNode>(shapes, mid, end);
        }
    }

    bool Hit(const Ray &ray, HitResult &hit, double t_min, double t_max) const override
    {
        if (!bbox.Hit(ray, t_min, t_max))
            return false;

        auto h1 = left->Hit(ray, hit, t_min, t_max);
        auto h2 = right->Hit(ray, hit, t_min, h1 ? hit.t : t_max);

        return h1 || h2;
    }

    AABB BoundingBox() const override { return bbox; }

private:
    shared_ptr<Hittable> left;
    shared_ptr<Hittable> right;
    AABB bbox;

    static bool BoxCompare(const shared_ptr<Hittable> a, const shared_ptr<Hittable> b, int axis_index)
    {
        auto a_axis_interval = a->BoundingBox().AxisInterval(axis_index);
        auto b_axis_interval = b->BoundingBox().AxisInterval(axis_index);
        return a_axis_interval.min < b_axis_interval.min;
    }

    static bool BoxCompare_X(const shared_ptr<Hittable> a, const shared_ptr<Hittable> b)
    {
        return BoxCompare(a, b, 0);
    }

    static bool BoxCompare_Y(const shared_ptr<Hittable> a, const shared_ptr<Hittable> b)
    {
        return BoxCompare(a, b, 1);
    }

    static bool BoxCompare_Z(const shared_ptr<Hittable> a, const shared_ptr<Hittable> b)
    {
        return BoxCompare(a, b, 2);
    }
};
