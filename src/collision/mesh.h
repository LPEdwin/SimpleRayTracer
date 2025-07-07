#pragma once

#include "core/vector3.h"
#include "core/hittable.h"
#include "collision/face.h"

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
        bbox = CalculateBoundingBoxFromFaces(faces);
    }

    Mesh(std::vector<Face> &&faces, std::shared_ptr<Material> material = DefaultMaterial())
        : faces(std::move(faces)), material(material)
    {
        bbox = CalculateBoundingBoxFromFaces(faces);
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
            if (HitFace(ray, face, t))
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
};
