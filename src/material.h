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
    Metal(const Color &albedo) : albedo(albedo) {}

    bool Scatter(const Ray &ray_in, const HitResult &hit, Color &attenuation, Ray &ray_out) const override
    {
        auto reflected = Reflect(ray_in.direction, hit.normal);
        ray_out = Ray(hit.point, reflected);
        attenuation = albedo;
        return true;
    }

private:
    Color albedo;
};

inline std::shared_ptr<Material> DefaultMaterial()
{
    static std::shared_ptr<Material> default_mat = std::make_shared<Lambertian>(Color(0.5, 0.5, 0.5));
    return default_mat;
}
