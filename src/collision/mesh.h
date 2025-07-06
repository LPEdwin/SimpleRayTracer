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

inline static bool IntersectRayFace(const Ray &ray, const Face &face, double &t)
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

class Mesh : public Hittable
{
private:
    std::vector<Face> faces;
    std::shared_ptr<Material> material;
    AABB bbox;

public:
    Mesh(const std::vector<Face> &faces, std::shared_ptr<Material> material = DefaultMaterial())
        : faces(faces), material(material)
    {
        SetBoundingBox();
    }

    Mesh(std::vector<Face> &&faces, std::shared_ptr<Material> material = DefaultMaterial())
        : faces(std::move(faces)), material(material)
    {
        SetBoundingBox();
    }

    size_t FaceCount() const
    {
        return faces.size();
    }
    
    bool Hit(const Ray &ray, HitResult &hitResult, double t_min, double t_max) const override
    {
        double t;
        bool hasHit = false;
        HitResult tmpHit;
        for (const auto &face : faces)
        {
            if (IntersectRayFace(ray, face, t))
            {
                if (t < t_max && t > t_min)
                {
                    t_max = t;
                    hasHit = true;

                    tmpHit.point = ray.At(t);
                    tmpHit.t = t;
                    tmpHit.normal = face.normal;
                }
            }
        }
        if (hasHit)
        {
            hitResult = tmpHit;
            hitResult.SetFaceNormal(ray, hitResult.normal);
            hitResult.material = material;
        }
        return hasHit;
    }

    virtual AABB BoundingBox() const override
    {
        return bbox;
    }

private:
    void SetBoundingBox()
    {
        if (faces.empty())
            bbox = AABB();

        Vector3 minPoint = Vector3::Min(faces[0].v0, Vector3::Min(faces[0].v1, faces[0].v2));
        Vector3 maxPoint = Vector3::Max(faces[0].v0, Vector3::Max(faces[0].v1, faces[0].v2));

        for (const auto &face : faces)
        {
            minPoint = Vector3::Min(minPoint, Vector3::Min(face.v0, Vector3::Min(face.v1, face.v2)));
            maxPoint = Vector3::Max(maxPoint, Vector3::Max(face.v0, Vector3::Max(face.v1, face.v2)));
        }

        bbox = AABB(minPoint, maxPoint);
    }
};