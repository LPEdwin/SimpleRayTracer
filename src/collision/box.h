#pragma once

#include <memory>

#include "hittable_list.h"
#include "quad.h"

inline shared_ptr<HittableList> CreateBox(const Point3 &a, const Point3 &b, shared_ptr<Material> mat)
{
    // Returns the 3D box (six sides) that contains the two opposite vertices a & b.

    auto sides = make_shared<HittableList>();

    // Construct the two opposite vertices with the minimum and maximum coordinates.
    auto min = Point3(std::fmin(a.x(), b.x()), std::fmin(a.y(), b.y()), std::fmin(a.z(), b.z()));
    auto max = Point3(std::fmax(a.x(), b.x()), std::fmax(a.y(), b.y()), std::fmax(a.z(), b.z()));

    auto dx = Vector3(max.x() - min.x(), 0, 0);
    auto dy = Vector3(0, max.y() - min.y(), 0);
    auto dz = Vector3(0, 0, max.z() - min.z());

    sides->add(make_shared<Quad>(Point3(min.x(), min.y(), max.z()), dx, dy, mat));  // front
    sides->add(make_shared<Quad>(Point3(max.x(), min.y(), max.z()), -dz, dy, mat)); // right
    sides->add(make_shared<Quad>(Point3(max.x(), min.y(), min.z()), -dx, dy, mat)); // back
    sides->add(make_shared<Quad>(Point3(min.x(), min.y(), min.z()), dz, dy, mat));  // left
    sides->add(make_shared<Quad>(Point3(min.x(), max.y(), max.z()), dx, -dz, mat)); // top
    sides->add(make_shared<Quad>(Point3(min.x(), min.y(), min.z()), dx, dz, mat));  // bottom

    return sides;
}
