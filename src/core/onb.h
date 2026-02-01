#pragma once

#include "vector3.h"

class Onb
{
public:
    Onb(const Vector3 &n)
    {
        axis[2] = UnitVector(n);
        Vector3 a;
        if (std::fabs(w().x()) > 0.9)
            a = Vector3(0, 1, 0);
        else
            a = Vector3(1, 0, 0);
        axis[1] = UnitVector(Cross(w(), a));
        axis[0] = Cross(w(), v());
    }

    Vector3 u() const { return axis[0]; }
    Vector3 v() const { return axis[1]; }
    Vector3 w() const { return axis[2]; }

    Vector3 Transform(const Vector3 &a) const
    {
        return a.x() * u() + a.y() * v() + a.z() * w();
    }

private:
    Vector3 axis[3];
};