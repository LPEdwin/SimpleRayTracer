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

#include "ray.h"
#include "vector3.h"
#include "hittable.h"

class Material
{
public:
    virtual ~Material() = default;
    virtual bool Scatter(const Ray &ray_in, const HitResult &hit, Color &attenuation, Ray &ray_out) const = 0;
};

// A material that reflects light diffusely preferred in normal directiron by following Lambert's cosine law.
class Lambertian : public Material
{
public:
    Lambertian(const Color &albedo) : albedo(albedo) {}

    bool Scatter(const Ray &ray_in, const HitResult &hit, Color &attenuation, Ray &ray_out) const override
    {
        auto scatter_direction = hit.normal + RandomUnitVector();

        if (scatter_direction.NearZero())
            scatter_direction = hit.normal;

        ray_out = Ray(hit.point, scatter_direction);
        attenuation = albedo;
        return true;
    }

private:
    Color albedo;
};

class Metal : public Material
{
public:
    Metal(const Color &albedo, double fuzziness = 0.0) : albedo(albedo), fuzziness(fuzziness) {}

    bool Scatter(const Ray &ray_in, const HitResult &hit, Color &attenuation, Ray &ray_out) const override
    {
        auto reflected = Reflect(ray_in.direction, hit.normal);
        reflected = UnitVector(reflected) + (fuzziness * RandomUnitVector());
        ray_out = Ray(hit.point, reflected);
        attenuation = albedo;
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

    bool Scatter(const Ray &ray_in, const HitResult &hit, Color &attenuation, Ray &ray_out) const override
    {
        attenuation = Color(1.0, 1.0, 1.0);
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

        ray_out = Ray(hit.point, direction);
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
