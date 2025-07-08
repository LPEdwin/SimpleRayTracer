
#pragma once

#include "core/vector3.h"
#include "core/hittable.h"

struct Face
{
    Point3 v0, v1, v2;
    Vector3 normal;

    Face(Point3 v0, Point3 v1, Point3 v2)
        : v0(v0), v1(v1), v2(v2), normal(UnitVector(Cross(v1 - v0, v2 - v0))) {}
};

inline bool HitFace(const Ray &ray, const Face &face, double &t)
{
    constexpr double EPS = 1e-8;

    const Vector3 edge1 = face.v1 - face.v0;
    const Vector3 edge2 = face.v2 - face.v0;

    const Vector3 h = Cross(ray.direction, edge2);
    const double a = Dot(edge1, h);

    if (std::abs(a) < EPS)
        return false; // Ray is parallel to triangle

    const double f = 1.0 / a;
    const Vector3 s = ray.origin - face.v0;
    const double u = f * Dot(s, h);

    if (u < 0.0 || u > 1.0)
        return false;

    const Vector3 q = Cross(s, edge1);
    const double v = f * Dot(ray.direction, q);

    if (v < 0.0 || (u + v) > 1.0)
        return false;

    t = f * Dot(edge2, q);

    return true;
}