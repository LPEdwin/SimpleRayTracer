#pragma once

#include <memory>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>

#include "core/hittable.h"
#include "core/transform.h"
#include "collision/triangle.h"

static std::vector<Triangle> LoadObjectFile(const std::string &filename)
{
    std::vector<Triangle> faces;
    std::vector<Vector3> vertices;

    std::ifstream file(filename);
    if (!file)
    {
        throw std::runtime_error("Failed to open file: " + filename);
    }

    std::string line;
    while (std::getline(file, line))
    {
        if (line.empty() || line[0] == '#')
            continue;

        std::istringstream iss(line);
        std::string prefix;
        iss >> prefix;

        if (prefix == "v")
        {
            double x, y, z;
            if (!(iss >> x >> y >> z))
            {
                throw std::runtime_error("Invalid vertex format in line: " + line);
            }
            vertices.emplace_back(Vector3(x, y, z));
        }
        else if (prefix == "f")
        {
            std::string v1_str, v2_str, v3_str;
            if (!(iss >> v1_str >> v2_str >> v3_str))
            {
                throw std::runtime_error("Invalid face format in line: " + line);
            }

            // Parse vertex indices (handle formats like "1", "1/2", "1/2/3")
            auto parseVertexIndex = [](const std::string &str) -> int
            {
                size_t slash_pos = str.find('/');
                if (slash_pos != std::string::npos)
                    return std::stoi(str.substr(0, slash_pos)) - 1; // Convert to 0-indexed
                return std::stoi(str) - 1;                          // Convert to 0-indexed
            };

            int v1 = parseVertexIndex(v1_str);
            int v2 = parseVertexIndex(v2_str);
            int v3 = parseVertexIndex(v3_str);

            if (v1 < 0 || v1 >= vertices.size() ||
                v2 < 0 || v2 >= vertices.size() ||
                v3 < 0 || v3 >= vertices.size())
            {
                throw std::runtime_error("Face references invalid vertex indices");
            }

            faces.emplace_back(Triangle(vertices[v1], vertices[v2], vertices[v3]));
        }
    }

    if (faces.empty())
    {
        throw std::runtime_error("No valid faces found in OBJ file");
    }

    return faces;
}

std::vector<std::shared_ptr<Hittable>> AsHittables(const std::vector<Triangle> &triangles)
{
    std::vector<std::shared_ptr<Hittable>> hittables;
    hittables.reserve(triangles.size());

    for (const auto &tri : triangles)
    {
        hittables.push_back(std::make_shared<Triangle>(tri));
    }
    return hittables;
}