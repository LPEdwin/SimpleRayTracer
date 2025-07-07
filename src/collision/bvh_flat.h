#pragma once

#include <vector>
#include <stack>

#include "core/ray.h"
#include "core/hittable.h"
#include "collision/mesh.h"
#include "collision/face.h"

struct BvhFlatNode
{
    Vector3 min;
    Vector3 max;

    size_t left_index = static_cast<size_t>(-1);
    size_t right_index = static_cast<size_t>(-1);

    size_t object_index = static_cast<size_t>(-1);
};

bool HitBvhFlatNode(
    const BvhFlatNode &node,
    const Point3 &ray_orig,
    const Vector3 &ray_dir,
    double t_min,
    double t_max)
{

    for (int axis = 0; axis < 3; axis++)
    {
        // check if the ray is parallel to the axis
        if (std::abs(ray_dir[axis]) < 1e-8)
        {
            if (ray_orig[axis] < node.min[axis] || ray_orig[axis] > node.max[axis])
                return false;
            continue;
        }
        const double adinv = 1.0 / ray_dir[axis];

        auto t0 = (node.min[axis] - ray_orig[axis]) * adinv;
        auto t1 = (node.max[axis] - ray_orig[axis]) * adinv;

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

bool TraverseFlatBvh(
    const Ray &ray,
    HitResult &hit,
    double t_min,
    double t_max,
    const std::vector<BvhFlatNode> &nodes,
    const std::vector<Face> &faces)
{
    std::vector<size_t> stack;
    stack.reserve(128);
    stack.push_back(0);
    bool hasHit(false);

    while (!stack.empty())
    {
        auto node = nodes[stack.back()];
        stack.pop_back();

        const auto isLeaf = node.object_index != static_cast<size_t>(-1);

        // check if nodes bounding box  is hit
        if (!HitBvhFlatNode(node, ray.origin, ray.direction, t_min, t_max))
            continue;

        // it's a leaf
        if (isLeaf)
        {
            auto face = faces[node.object_index];
            double t = 0.0;
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
        else
        {
            stack.push_back(node.left_index);
            stack.push_back(node.right_index);
        }
    }

    return hasHit;
}

struct AABBHelper
{
    Vector3 min;
    Vector3 max;
};

AABBHelper FromFace(const Face &face)
{
    constexpr double delta = 1e-4;
    AABBHelper bbox;
    bbox.min = Vector3::Min(face.v0, Vector3::Min(face.v1, face.v2));
    bbox.max = Vector3::Max(face.v0, Vector3::Max(face.v1, face.v2));
    for (auto i = 0; i < 3; ++i)
    {
        if (std::abs(bbox.max[i] - bbox.min[i]) < delta)
        {
            // If the bounding box is too thin in this dimension, expand it slightly
            bbox.min[i] -= delta;
            bbox.max[i] += delta;
        }
    }
    return bbox;
}

AABBHelper Union(const AABBHelper &a, const AABBHelper &b)
{
    AABBHelper bbox;
    bbox.min = Vector3::Min(a.min, b.min);
    bbox.max = Vector3::Max(a.max, b.max);
    return bbox;
}

AABBHelper Union(const std::vector<Face> &faces, size_t begin, size_t end)
{
    AABBHelper bbox;
    if (begin >= end)
        throw std::invalid_argument("Cannot compute bounding box of empty range");

    bbox = FromFace(faces[begin]);

    for (size_t i = begin + 1; i < end; ++i)
        bbox = Union(bbox, FromFace(faces[i]));

    return bbox;
}

struct BoxCompareFlat
{
    int index;
    BoxCompareFlat(int idx) : index(idx) {}

    bool operator()(const Face &a, const Face &b) const
    {
        auto a_axis_interval = FromFace(a);
        auto b_axis_interval = FromFace(b);
        return a_axis_interval.min[index] < b_axis_interval.min[index];
    }
};

std::vector<BvhFlatNode> BuildFlatBvh(std::vector<Face> &faces)
{
    std::vector<BvhFlatNode> nodes;

    struct BuildEntry
    {
        size_t begin, end;
        size_t parentIndex;
        bool isLeftChild;
    };

    std::stack<BuildEntry> stack;
    stack.push({0, faces.size(), static_cast<size_t>(-1), false});

    while (!stack.empty())
    {
        BuildEntry entry = stack.top();
        stack.pop();

        size_t begin = entry.begin;
        size_t end = entry.end;
        size_t count = end - begin;

        AABBHelper bbox = Union(faces, begin, end);
        BvhFlatNode node = {
            .min = bbox.min,
            .max = bbox.max,
            .left_index = static_cast<size_t>(-1),
            .right_index = static_cast<size_t>(-1),
            .object_index = static_cast<size_t>(-1)};

        size_t nodeIndex = nodes.size();
        nodes.push_back(node);

        if (entry.parentIndex != static_cast<size_t>(-1))
        {
            if (entry.isLeftChild)
                nodes[entry.parentIndex].left_index = nodeIndex;
            else
                nodes[entry.parentIndex].right_index = nodeIndex;
        }

        if (count == 1)
        {
            nodes[nodeIndex].object_index = begin;
            continue;
        }

        // Choose longest axis as split axis
        Vector3 extent = bbox.max - bbox.min;
        int axis = (extent.x() > extent.y() && extent.x() > extent.z()) ? 0 : (extent.y() > extent.z() ? 1 : 2);

        // Partition around middle
        size_t mid = begin + count / 2;
        // This function is an optimization. It doesn't sort the whole array.
        // It only reorders the elements such that all elements left from the n-th element
        // are less than the n-th element and vice versa.
        // std::nth_element(
        //     faces.begin() + begin, faces.begin() + mid, faces.begin() + end,
        //     [axis](const Face &a, const Face &b)
        //     {
        //         double a_centroid = (a.v0[axis] + a.v1[axis] + a.v2[axis]) / 3.0;
        //         double b_centroid = (b.v0[axis] + b.v1[axis] + b.v2[axis]) / 3.0;
        //         return a_centroid < b_centroid;
        //     });

        std::sort(faces.begin() + begin, faces.begin() + end, BoxCompareFlat(axis));
        // std::nth_element(faces.begin() + begin, faces.begin() + mid, faces.begin() + end, BoxCompareFlat(axis));

        // Push children in reverse order (right first) so left is on top for better memory layout.
        stack.push({mid, end, nodeIndex, false});
        stack.push({begin, mid, nodeIndex, true});
    }

    return nodes;
}