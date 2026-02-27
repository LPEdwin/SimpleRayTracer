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

class MixtureSampling : public SamplingPdf
{
public:
    MixtureSampling(std::shared_ptr<SamplingPdf> sampler1, std::shared_ptr<SamplingPdf> sampler2)
        : sampler1(std::move(sampler1)), sampler2(std::move(sampler2)) {}

    virtual double PdfValue(const Vector3 &direction) const override
    {
        return p * sampler1->PdfValue(direction) + (1 - p) * sampler2->PdfValue(direction);
    }

    virtual Vector3 GenerateDirection() const override
    {
        if (RandomDouble() < p)
        {
            return sampler1->GenerateDirection();
        }
        else
        {
            return sampler2->GenerateDirection();
        }
    }

private:
    double p = 0.5;
    std::shared_ptr<SamplingPdf> sampler1;
    std::shared_ptr<SamplingPdf> sampler2;
};