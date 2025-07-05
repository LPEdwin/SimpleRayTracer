#pragma once

#include <memory>

#include "core/vector3.h"
#include "core/ray.h"

class EnvironmentMap
{
public:
    virtual Color GetColor(const Ray &ray) const = 0;
};

class GradientMap : public EnvironmentMap
{
public:
    GradientMap(const Vector3 &bottomColor, const Vector3 &topColor)
        : bottomColor(bottomColor), topColor(topColor) {}

    virtual Color GetColor(const Ray &ray) const override
    {
        auto dir = UnitVector(ray.direction);
        float t = 0.5f * (dir.y() + 1.0f);
        return (1.0f - t) * bottomColor + t * topColor;
    }

    static std::shared_ptr<GradientMap> Sky()
    {
        static const auto sky = std::make_shared<GradientMap>(Color(1.0, 1.0, 1.0), Color(0.5, 0.7, 1.0));
        return sky;
    }

private:
    Vector3 bottomColor;
    Vector3 topColor;
};
