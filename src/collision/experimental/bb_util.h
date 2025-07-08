#pragma once

#include <vector>
#include <stack>

#include "core/ray.h"
#include "core/hittable.h"
#include "collision/face.h"

AABB CalculateBoundingBoxFromFaces(const vector<Face> &faces)
{
    if (faces.empty())
        return AABB();

    Vector3 minPoint = Vector3::Min(faces[0].v0, Vector3::Min(faces[0].v1, faces[0].v2));
    Vector3 maxPoint = Vector3::Max(faces[0].v0, Vector3::Max(faces[0].v1, faces[0].v2));

    for (const auto &face : faces)
    {
        minPoint = Vector3::Min(minPoint, Vector3::Min(face.v0, Vector3::Min(face.v1, face.v2)));
        maxPoint = Vector3::Max(maxPoint, Vector3::Max(face.v0, Vector3::Max(face.v1, face.v2)));
    }

    return AABB(minPoint, maxPoint);
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

struct BBCompare
{
    int index;
    BBCompare(int idx) : index(idx) {}

    bool operator()(const Face &a, const Face &b) const
    {
        auto a_axis_interval = FromFace(a);
        auto b_axis_interval = FromFace(b);
        return a_axis_interval.min[index] < b_axis_interval.min[index];
    }
};