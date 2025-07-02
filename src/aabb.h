#pragma once

#include "interval.h"
#include "vector3.h"
#include "ray.h"

class AABB
{
public:
    Interval x, y, z;

    AABB() {}

    AABB(const Interval &x, const Interval &y, const Interval &z)
        : x(x), y(y), z(z)
    {
        PadToMinimums();
    }

    AABB(const Point3 &a, const Point3 &b)
    {
        // Treat the two points a and b as extrema for the bounding box, so we don't require a
        // particular minimum/maximum coordinate order.

        x = (a[0] <= b[0]) ? Interval(a[0], b[0]) : Interval(b[0], a[0]);
        y = (a[1] <= b[1]) ? Interval(a[1], b[1]) : Interval(b[1], a[1]);
        z = (a[2] <= b[2]) ? Interval(a[2], b[2]) : Interval(b[2], a[2]);

        PadToMinimums();
    }

    AABB(const AABB &box0, const AABB &box1)
    {
        x = Interval(box0.x, box1.x);
        y = Interval(box0.y, box1.y);
        z = Interval(box0.z, box1.z);
    }

    const Interval &AxisInterval(int n) const
    {
        if (n == 1)
            return y;
        if (n == 2)
            return z;
        return x;
    }

    bool Hit(const Ray &r, double t_min, double t_max) const
    {
        const Point3 &ray_orig = r.origin;
        const Vector3 &ray_dir = r.direction;

        for (int axis = 0; axis < 3; axis++)
        {
            const Interval &ax = AxisInterval(axis);
            const double adinv = 1.0 / ray_dir[axis];

            auto t0 = (ax.min - ray_orig[axis]) * adinv;
            auto t1 = (ax.max - ray_orig[axis]) * adinv;

            if (t0 < t1)
            {
                if (t0 > t_min)
                    t_min = t0;
                if (t1 < t_max)
                    t_max = t1;
            }
            else
            {
                if (t1 > t_min)
                    t_min = t1;
                if (t0 < t_max)
                    t_max = t0;
            }

            if (t_max <= t_min)
                return false;
        }
        return true;
    }

    // Returns the index of the longest axis of the bounding box.
    int LongestAxis() const
    {
        if (x.Length() > y.Length())
            return x.Length() > z.Length() ? 0 : 2;
        else
            return y.Length() > z.Length() ? 1 : 2;
    }

    static const AABB empty, universe;

private:
    // Adjust the AABB so that no side is narrower than some delta, padding if necessary.
    void PadToMinimums()
    {
        double delta = 0.0001;
        if (x.Length() < delta)
            x = x.Expanded(delta);
        if (y.Length() < delta)
            y = y.Expanded(delta);
        if (z.Length() < delta)
            z = z.Expanded(delta);
    }
};

const AABB AABB::empty = AABB(Interval::kEmpty, Interval::kEmpty, Interval::kEmpty);
const AABB AABB::universe = AABB(Interval::kUnbounded, Interval::kUnbounded, Interval::kUnbounded);

AABB operator+(const AABB &bbox, const Vector3 &offset)
{
    return AABB(bbox.x + offset.x(), bbox.y + offset.y(), bbox.z + offset.z());
}

AABB operator+(const Vector3 &offset, const AABB &bbox)
{
    return bbox + offset;
}