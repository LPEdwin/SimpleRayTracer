//==============================================================================================
// Originally written in 2016 by Peter Shirley <ptrshrl@gmail.com>
//
// To the extent possible under law, the author(s) have dedicated all copyright and related and
// neighboring rights to this software to the public domain worldwide. This software is
// distributed without any warranty.
//
// You should have received a copy (see file COPYING.txt) of the CC0 Public Domain Dedication
// along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
//==============================================================================================

#pragma once

#include "core/ray.h"
#include "core/vector3.h"
#include "core/hittable.h"
#include "core/onb.h"
#include "core/sampling_pdf.h"

struct ScatterResult
{
    Color Attenuation;
    std::shared_ptr<SamplingPdf> Sampler;
    bool IsDeltaDistribution;
    Ray RayOut;
};

class Material
{
public:
    virtual ~Material() = default;

    virtual double Evaluate(const Ray &ray_in, const HitResult &rec, const Ray &ray_scattered) const
    {
        return 0.0;
    }

    virtual bool Scatter(const Ray &ray_in, const HitResult &hit, ScatterResult &scatter_out) const
    {
        return false;
    }

    virtual Color Emitted(const HitResult &hit, double u, double v) const
    {
        return Color(0.0, 0.0, 0.0);
    }
};

class Emissive : public Material
{
public:
    Emissive(const Color &emission) : emission(emission) {}

    virtual Color Emitted(const HitResult &hit, double u, double v) const override
    {
        if (!hit.front_face)
            return Color(0.0, 0.0, 0.0);
        return emission;
    }

private:
    Color emission;
};

// A material using cosine weighting for scattered rays with cosine importance samping.
class LambertianCosine : public Material
{
public:
    LambertianCosine(const Color &albedo) : albedo(albedo) {}

    double Evaluate(const Ray &ray_in, const HitResult &rec, const Ray &ray_scattered) const override
    {
        auto cos_theta = Dot(UnitVector(rec.normal), UnitVector(ray_scattered.direction));
        return cos_theta < 0 ? 0 : cos_theta / std::numbers::pi;
    }

    bool Scatter(const Ray &ray_in, const HitResult &hit, ScatterResult &scatter_out) const override
    {
        scatter_out.Sampler = make_shared<CosineHemisphereSampling>(hit.normal);
        scatter_out.IsDeltaDistribution = false;
        scatter_out.Attenuation = albedo;
        return true;
    }

private:
    Color albedo;
};

// A material using cosine weighting for scattered rays with uniform samping.
class LambertianUniform : public Material
{
public:
    LambertianUniform(const Color &albedo) : albedo(albedo) {}

    double Evaluate(const Ray &ray_in, const HitResult &rec, const Ray &ray_scattered) const override
    {
        auto cos_theta = Dot(UnitVector(rec.normal), UnitVector(ray_scattered.direction));
        return cos_theta < 0 ? 0 : cos_theta / std::numbers::pi;
    }

    bool Scatter(const Ray &ray_in, const HitResult &hit, ScatterResult &scatter_out) const override
    {
        scatter_out.Sampler = make_shared<HemisphereSampling>(hit.normal);
        scatter_out.IsDeltaDistribution = false;
        scatter_out.Attenuation = albedo;
        return true;
    }

private:
    Color albedo;
};

class Lambertian : public LambertianCosine
{
public:
    using LambertianCosine::LambertianCosine;
};

class Metal : public Material
{
public:
    Metal(const Color &albedo, double fuzziness = 0.0) : albedo(albedo), fuzziness(fuzziness) {}

    bool Scatter(const Ray &ray_in, const HitResult &hit, ScatterResult &scatter_out) const override
    {
        auto reflected = Reflect(ray_in.direction, hit.normal);
        reflected = UnitVector(reflected) + (fuzziness * RandomUnitVector());

        scatter_out.Sampler = nullptr;
        scatter_out.IsDeltaDistribution = true;
        scatter_out.RayOut = Ray(hit.point, reflected, ray_in.time);
        scatter_out.Attenuation = albedo;
        return (Dot(reflected, hit.normal) > 0);
    }

private:
    Color albedo;
    double fuzziness;
};

inline std::shared_ptr<Material> DefaultMaterial()
{
    static std::shared_ptr<Material> default_mat = std::make_shared<Lambertian>(Color(0.5, 0.5, 0.5));
    return default_mat;
}

class Dielectric : public Material
{
public:
    Dielectric(double refraction_index) : refraction_index(refraction_index) {}

    bool Scatter(const Ray &ray_in, const HitResult &hit, ScatterResult &scatter_out) const override
    {
        scatter_out.Attenuation = Color(1.0, 1.0, 1.0);
        scatter_out.Sampler = nullptr;
        scatter_out.IsDeltaDistribution = true;

        double ri = hit.front_face ? (1.0 / refraction_index) : refraction_index;

        Vector3 unit_direction = UnitVector(ray_in.direction);
        double cos_theta = std::fmin(Dot(-unit_direction, hit.normal), 1.0);
        double sin_theta = std::sqrt(1.0 - cos_theta * cos_theta);

        bool cannot_refract = ri * sin_theta > 1.0;
        Vector3 direction;

        if (cannot_refract || Reflectance(cos_theta, ri) > RandomDouble())
            direction = Reflect(unit_direction, hit.normal);
        else
            direction = Refract(unit_direction, hit.normal, ri);

        scatter_out.RayOut = Ray(hit.point, direction, ray_in.time);
        return true;
    }

private:
    // Refractive index in vacuum or air, or the ratio of the material's refractive index over
    // the refractive index of the enclosing media
    double refraction_index;

    static double Reflectance(double cosine, double refraction_index)
    {
        // Use Schlick's approximation for reflectance.
        auto r0 = (1 - refraction_index) / (1 + refraction_index);
        r0 = r0 * r0;
        return r0 + (1 - r0) * std::pow((1 - cosine), 5);
    }
};
