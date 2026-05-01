#pragma once

#include <numbers>
#include <cmath>
#include "vector3.h"
#include "onb.h"
#include "collision/quad.h"

class SamplingPdf
{
public:
    virtual ~SamplingPdf() = default;

    virtual double PdfValue(const Vector3 &direction) const = 0;
    virtual Vector3 GenerateDirection() const = 0;
};

class SphereSampling : public SamplingPdf
{
public:
    virtual double PdfValue(const Vector3 &direction) const override
    {
        return 1.0 / (4.0 * std::numbers::pi);
    }

    virtual Vector3 GenerateDirection() const override
    {
        return RandomUnitVector();
    }
};

class HemisphereSampling : public SamplingPdf
{
public:
    HemisphereSampling(const Vector3 &w) : w(w) {}

    virtual double PdfValue(const Vector3 &direction) const override
    {
        return 1.0 / (2.0 * std::numbers::pi);
    }

    virtual Vector3 GenerateDirection() const override
    {
        return RandomOnHemisphere(w);
    }

private:
    Vector3 w;
};

class CosineHemisphereSampling : public SamplingPdf
{
public:
    CosineHemisphereSampling(const Vector3 &w) : uvw(w) {}

    virtual double PdfValue(const Vector3 &direction) const override
    {
        auto cosine_theta = Dot(UnitVector(direction), uvw.w());
        return std::fmax(0, cosine_theta / std::numbers::pi);
    }

    virtual Vector3 GenerateDirection() const override
    {
        return uvw.Transform(RandomCosineDirection());
    }

private:
    Onb uvw;
};

class HittableSampling : public SamplingPdf
{
public:
    HittableSampling(const Hittable &objects, const Point3 &origin) : objects(objects), origin(origin) {}

    virtual double PdfValue(const Vector3 &direction) const override
    {
        return objects.PdfValue(origin, direction);
    }

    virtual Vector3 GenerateDirection() const override
    {
        return objects.SampleRandomPoint(origin);
    }

private:
    const Hittable &objects;
    Point3 origin;
};

template <typename tptr>
class MixtureSamplingT : public SamplingPdf
{
public:
    MixtureSamplingT(std::vector<tptr> samplers)
        : samplers(std::move(samplers))
    {
        if (this->samplers.empty())
            throw std::invalid_argument("Argument samplers can't be empty.");
    }

    virtual double PdfValue(const Vector3 &direction) const override
    {
        auto sum = 0.0;
        for (const auto &s : samplers)
            sum += s->PdfValue(direction);
        return sum / samplers.size();
    }

    virtual Vector3 GenerateDirection() const override
    {
        return samplers[RandomInt(0, samplers.size() - 1)]->GenerateDirection();
    }

private:
    std::vector<tptr> samplers;
};

using MixtureSamplingSafe = MixtureSamplingT<std::shared_ptr<SamplingPdf>>;
using MixtureSampling = MixtureSamplingT<SamplingPdf *>;

std::shared_ptr<MixtureSamplingSafe> CreateLightSampler(const std::vector<Quad *> &lights, const Point3 &origin)
{
    std::vector<std::shared_ptr<SamplingPdf>> samplers;
    for (const auto &l : lights)
        samplers.push_back(std::make_shared<HittableSampling>(*l, origin));
    return std::make_shared<MixtureSamplingSafe>(samplers);
}