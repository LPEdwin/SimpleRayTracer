#pragma once

#include <vector>
#include <stack>
#include <iostream>

#include "core/ray.h"
#include "core/hittable.h"
#include "collision/mesh.h"
#include "collision/face.h"
#include "collision/triangle.h"
#include "collision/bvh_node.h"
#include "collision/bvh_flat.h"

void FlattenBvhTree(
    const std::shared_ptr<Hittable> &root,
    std::vector<BvhFlatNode> &flatNodes,
    std::vector<Face> &faceRefs,
    size_t &nodeIndexOut)
{
    size_t currentIndex = flatNodes.size();
    flatNodes.push_back(BvhFlatNode{}); // reserve slot

    // Initialize bounding box immediately
    flatNodes[currentIndex].min = root->BoundingBox().Min();
    flatNodes[currentIndex].max = root->BoundingBox().Max();

    if (auto node = std::dynamic_pointer_cast<BvhNode>(root))
    {
        static auto count = 0;
        cout << "Flattening BvhNode: " << count++ << endl;
        size_t leftIndex, rightIndex;

        FlattenBvhTree(node->LeftNode(), flatNodes, faceRefs, leftIndex);
        FlattenBvhTree(node->RightNode(), flatNodes, faceRefs, rightIndex);

        // Get fresh reference after recursive calls (vector may have reallocated)
        flatNodes[currentIndex].left_index = leftIndex;
        flatNodes[currentIndex].right_index = rightIndex;
        flatNodes[currentIndex].object_index = static_cast<size_t>(-1); // internal node
    }
    else if (auto node = std::dynamic_pointer_cast<Triangle>(root))
    {
        faceRefs.push_back(Face(node->v0, node->v1, node->v2));
        flatNodes[currentIndex].object_index = faceRefs.size() - 1;
        flatNodes[currentIndex].left_index = flatNodes[currentIndex].right_index = static_cast<size_t>(-1);
    }
    else
    {
        throw std::runtime_error("Unsupported Hittable type for flattening");
    }
    nodeIndexOut = currentIndex;
}

std::vector<BvhFlatNode> FlattenBvh(const std::shared_ptr<Hittable> &root, std::vector<Face> &faceRefs)
{
    std::vector<BvhFlatNode> flatNodes;
    size_t nodeIndex = 0;
    FlattenBvhTree(root, flatNodes, faceRefs, nodeIndex);
    return flatNodes;
}