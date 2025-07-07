#pragma once

#include "core/vector3.h"
#include "core/hittable.h"
#include "collision/mesh.h"
#include "collision/bvh_flat.h"
#include "core/material.h"
#include "io/object_loader.h"
#include "collision/flat_bvh_util.h"

class Mesh2 : public Hittable
{
private:
    std::vector<Face> faces;
    std::vector<BvhFlatNode> bvhNodes;
    std::shared_ptr<Material> material;
    AABB bbox;

    Mesh2(std::vector<Face> &&faces, std::vector<BvhFlatNode> &&bvhNodes, AABB bbox, std::shared_ptr<Material> material = DefaultMaterial())
        : faces(std::move(faces)), bvhNodes(std::move(bvhNodes)), bbox(bbox), material(material)
    {
    }

public:
    static shared_ptr<Mesh2> Create(const std::string &file, std::shared_ptr<Material> material = DefaultMaterial())
    {
        std::vector<Face> faces = ReadFaces(file);
        if (faces.empty())
        {
            throw std::runtime_error("No valid faces found in OBJ file: " + file);
        }

        auto bvhNodes = BuildFlatBvh(faces);
        auto root = bvhNodes[0];
        AABB bbox(root.min, root.max);
        return shared_ptr<Mesh2>(new Mesh2(std::move(faces), std::move(bvhNodes), bbox, material));
    }

    static shared_ptr<Mesh2> CreateWithFlattenBvh(const std::string &file, std::shared_ptr<Material> material = DefaultMaterial())
    {
        auto triangles = LoadAsHittableList(file);
        auto bvh = BvhNode::Build(triangles->shapes);
        vector<Face> faces{};
        auto bvh_flat = FlattenBvh(bvh, faces);
        auto root = bvh_flat[0];
        AABB bbox(root.min, root.max);
        return shared_ptr<Mesh2>(new Mesh2(std::move(faces), std::move(bvh_flat), bbox, material));
    }

    size_t FaceCount() const
    {
        return faces.size();
    }

    size_t BvhNodeCount() const
    {
        return bvhNodes.size();
    }

    bool Hit(const Ray &ray, HitResult &hit, double t_min, double t_max) const override
    {
        if (TraverseFlatBvh(ray, hit, t_min, t_max, bvhNodes, faces))
        {
            hit.material = material;
            return true;
        }
        return false;
    }

    virtual AABB BoundingBox() const override
    {
        return bbox;
    }
};
