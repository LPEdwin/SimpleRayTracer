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

#include "core/aabb.h"
#include "core/hittable.h"
#include "collision/hittable_list.h"

#include <algorithm>

// Bounding Volume Hierachy
class BvhNode : public Hittable
{

public:
    static shared_ptr<BvhNode> Build(std::vector<shared_ptr<Hittable>> shapes)
    {
        if (shapes.empty())
        {
            throw std::runtime_error("BvhNode::Build: cannot build BVH from empty shape list.");
        }
        return BuildRecursive(shapes, 0, shapes.size());
    }

    static shared_ptr<BvhNode> Build(std::vector<shared_ptr<Hittable>> shapes, size_t start, size_t end)
    {
        if (start >= end)
        {
            throw std::runtime_error("BvhNode::Build: invalid span (start >= end).");
        }
        return BuildRecursive(shapes, start, end);
    }

private:
    BvhNode(shared_ptr<Hittable> left, shared_ptr<Hittable> right, AABB bbox)
        // standard pattern to avoid creating another copy of the parameters
        : left(std::move(left)), right(std::move(right)), bbox(std::move(bbox))
    {
    }

    static shared_ptr<BvhNode> BuildRecursive(std::vector<shared_ptr<Hittable>> &shapes, size_t start, size_t end)
    {
        // Build the bounding box of the span of source objects.
        auto bbox = AABB::empty;
        for (size_t i = start; i < end; i++)
            bbox = AABB(bbox, shapes[i]->BoundingBox());

        size_t object_span = end - start;

        shared_ptr<Hittable> left, right;

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
            std::sort(std::begin(shapes) + start, std::begin(shapes) + end, BoxCompare(bbox.LongestAxisIndex()));

            auto mid = start + object_span / 2;
            left = BuildRecursive(shapes, start, mid);
            right = BuildRecursive(shapes, mid, end);
        }
        return std::shared_ptr<BvhNode>(new BvhNode(left, right, bbox)); // can use make_shared because the constructor is private
    }

public:
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

    struct BoxCompare
    {
        int index;
        BoxCompare(int idx) : index(idx) {}

        // the object is now callable
        bool operator()(const shared_ptr<Hittable> &a, const shared_ptr<Hittable> &b) const
        {
            auto a_axis_interval = a->BoundingBox().AxisInterval(index);
            auto b_axis_interval = b->BoundingBox().AxisInterval(index);
            return a_axis_interval.min < b_axis_interval.min;
        }
    };
};
