#pragma once
//==============================================================================================
// To the extent possible under law, the author(s) have dedicated all copyright and related and
// neighboring rights to this software to the public domain worldwide. This software is
// distributed without any warranty.
//
// You should have received a copy (see file COPYING.txt) of the CC0 Public Domain Dedication
// along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
//==============================================================================================

#include "collision/hittable.h"
#include "hittable_list.h"
#include "ray.h"
#include "vector3.h"
#include "collision/aabb.h"
#include "material.h"

// actually quadrilateral primitive
class Quad : public Hittable
{
public:
    Quad(const Point3 &Q, const Vector3 &u, const Vector3 &v, shared_ptr<Material> mat)
        : Q(Q), u(u), v(v), mat(mat)
    {
        auto n = Cross(u, v);
        normal = UnitVector(n);
        D = Dot(normal, Q);
        w = n / Dot(n, n);

        auto bbox_diagonal1 = AABB(Q, Q + u + v);
        auto bbox_diagonal2 = AABB(Q + u, Q + v);
        bbox = AABB(bbox_diagonal1, bbox_diagonal2);
    }

    AABB BoundingBox() const override { return bbox; }

    bool Hit(const Ray &ray, HitResult &hit, double t_min, double t_max) const override
    {
        auto denom = Dot(normal, ray.direction);

        // No hit if the ray is parallel to the plane.
        if (std::fabs(denom) < 1e-8)
            return false;

        // Return false if the hit point parameter t is outside the ray interval.
        auto t = (D - Dot(normal, ray.origin)) / denom;
        if (t < t_min || t > t_max)
            return false;

        // Determine if the hit point lies within the planar shape using its plane coordinates.
        auto intersection = ray.At(t);
        Vector3 planar_hitpt_vector = intersection - Q;
        auto alpha = Dot(w, Cross(planar_hitpt_vector, v));
        auto beta = Dot(w, Cross(u, planar_hitpt_vector));

        if (alpha < 0 || beta < 0 || 1 < alpha || 1 < beta)
            return false;

        // Ray hits the 2D shape; set the rest of the hit record and return true.
        hit.t = t;
        hit.point = intersection;
        hit.material = mat;
        hit.SetFaceNormal(ray, normal);

        return true;
    }

private:
    Point3 Q;
    Vector3 u, v;
    Vector3 w;
    shared_ptr<Material> mat;
    AABB bbox;
    Vector3 normal;
    double D;
};