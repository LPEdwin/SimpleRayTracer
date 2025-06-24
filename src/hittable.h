#include "ray.h"
#include "vector3.h"

class HitResult
{
public:
    HitResult(const Point3 &point, const Vector3 &normal, double t) : point(point), normal(normal), t(t) {}

    Point3 point;
    Vector3 normal;
    double t;
};

class Hittable
{
public:
    ~Hittable() = default;
    virtual std::optional<HitResult> Hit(const Ray &ray, double t_min, double t_max) const = 0;
};
