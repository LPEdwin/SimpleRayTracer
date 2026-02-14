#pragma once

#include <numbers>
#include <cmath>
#include "vector3.h"
#include "onb.h"

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

class CosineSampling : public SamplingPdf
{
    CosineSampling(const Vector3 &w) : uvw(w) {}

public:
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