#pragma once

#include <vector>
#include <array>

#include "core/ray.h"
#include "core/hittable.h"
#include "collision/face.h"
#include "core/material.h"
#include "io/object_loader.h"
#include "collision/experimental/bb_util.h"

// Uses "final" types to prevent dynamic dispatching and raw pointers.
namespace StaticBvh
{
    static constexpr size_t INVALID_INDEX = static_cast<size_t>(-1);
    static constexpr size_t MAX_FACES_PER_LEAF = 8;

    struct FastBvhNode
    {
        Vector3 min;
        Vector3 max;

        FastBvhNode *leftNode = nullptr;
        FastBvhNode *rightNode = nullptr;

        std::array<size_t, MAX_FACES_PER_LEAF> faces;

        FastBvhNode()
        {
            faces.fill(INVALID_INDEX);
        }
    };

    bool Traverse(const Ray &ray, HitResult &hit, double &t_min, double &t_max, FastBvhNode *node, const std::vector<Face> &faces)
    {
        if (node == nullptr)
            return false;

        if (!HitAABB(node->min, node->max, ray.origin, ray.direction, t_min, t_max))
            return false;

        if (node->faces[0] != INVALID_INDEX)
        {
            double t;
            bool hasHit(false);
            for (size_t i = 0; i < node->faces.size(); i++)
            {
                if (node->faces[i] == INVALID_INDEX)
                    break;
                auto &face = faces[node->faces[i]];
                if (HitFace(ray, face, t))
                {
                    if (t >= t_min && t < t_max)
                    {
                        t_max = t;
                        hit.t = t;
                        hit.point = ray.At(t);
                        hit.normal = face.normal;
                        hit.SetFaceNormal(ray, face.normal);
                        hasHit = true;
                    }
                }
            }
            return hasHit;
        }

        bool hitLeft = false, hitRight = false;
        if (node->leftNode)
            hitLeft = Traverse(ray, hit, t_min, t_max, node->leftNode, faces);
        if (node->rightNode)
            hitRight = Traverse(ray, hit, t_min, t_max, node->rightNode, faces);
        return hitLeft || hitRight;
    }

    void BuildRecursive(FastBvhNode *node, std::vector<Face> &faces, size_t start, size_t end)
    {
        if (node == nullptr)
            throw std::invalid_argument("node can't be null.");

        FastBvhNode &n = *node;

        // current range length
        size_t count = end - start;

        if (count < 0)
            throw std::invalid_argument("count can't be negative.");

        // assign bounding box
        AABBHelper bbox = Union(faces, start, end);
        n.min = bbox.min;
        n.max = bbox.max;

        // if sparse enough add faces
        if (count <= MAX_FACES_PER_LEAF)
        {
            for (int i = 0; i < count; i++)
            {
                n.faces[i] = start + i;
            }
            return;
        }

        // else use longest axis as split axis
        Vector3 extent = n.max - n.min;
        int axisId = (extent.x() > extent.y() && extent.x() > extent.z()) ? 0 : (extent.y() > extent.z() ? 1 : 2);

        // sort and split faces
        size_t mid = start + count / 2;
        std::nth_element(faces.begin() + start, faces.begin() + mid, faces.begin() + end, BBCompareByMin(axisId));

        n.leftNode = new FastBvhNode();
        n.rightNode = new FastBvhNode();
        BuildRecursive(n.leftNode, faces, start, mid);
        BuildRecursive(n.rightNode, faces, mid, end);
    }

    FastBvhNode *Build(std::vector<Face> &faces)
    {
        if (faces.size() == 0)
            throw std::invalid_argument("faces can't be empty.");

        auto root = new FastBvhNode();
        BuildRecursive(root, faces, 0, faces.size());
        return root;
    }

    class Mesh : public Hittable
    {
    private:
        std::vector<Face> faces;
        FastBvhNode *root;
        std::shared_ptr<Material> material;
        AABB bbox;

        Mesh(FastBvhNode *root, std::vector<Face> &&faces, AABB bbox, std::shared_ptr<Material> material = DefaultMaterial())
            : root(root), faces(std::move(faces)), bbox(bbox), material(material)
        {
        }

    public:
        static std::shared_ptr<Mesh> Create(const std::string &file, std::shared_ptr<Material> material = DefaultMaterial())
        {
            std::vector<Face> faces = ReadFaces(file);
            if (faces.empty())
            {
                throw std::runtime_error("No valid faces found in OBJ file: " + file);
            }

            auto root = Build(faces);
            AABB bbox(root->min, root->max);
            return std::shared_ptr<Mesh>(new Mesh(root, std::move(faces), bbox, material));
        }

        size_t FaceCount() const
        {
            return faces.size();
        }

        bool Hit(const Ray &ray, HitResult &hit, double t_min, double t_max) const override
        {
            if (Traverse(ray, hit, t_min, t_max, root, faces))
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

        ~Mesh()
        {
            DestroyNode(root);
        }

    private:
        void DestroyNode(FastBvhNode *node)
        {
            if (node)
            {
                DestroyNode(node->leftNode);
                DestroyNode(node->rightNode);
                delete node;
            }
        }
    };
}