#pragma once

#include <array>
#include <numbers>

#include "vector3.h"

constexpr double DegToRad = std::numbers::pi / 180.0;

class Transform
{
private:
    std::array<std::array<double, 4>, 4> m;

public:
    // Default constructor creates identity matrix
    Transform()
    {
        for (int i = 0; i < 4; ++i)
        {
            for (int j = 0; j < 4; ++j)
            {
                m[i][j] = (i == j) ? 1.0 : 0.0;
            }
        }
    }

    // Constructor from 4x4 matrix values
    Transform(const std::array<std::array<double, 4>, 4> &matrix) : m(matrix) {}

    // Transform a point (assumes w = 1)
    Vector3 operator*(const Vector3 &point) const
    {
        double x = m[0][0] * point.x() + m[0][1] * point.y() + m[0][2] * point.z() + m[0][3];
        double y = m[1][0] * point.x() + m[1][1] * point.y() + m[1][2] * point.z() + m[1][3];
        double z = m[2][0] * point.x() + m[2][1] * point.y() + m[2][2] * point.z() + m[2][3];
        return Vector3(x, y, z);
    }

    // Transform a vector (assumes w = 0, no translation)
    Vector3 TransformVector(const Vector3 &vec) const
    {
        double x = m[0][0] * vec.x() + m[0][1] * vec.y() + m[0][2] * vec.z();
        double y = m[1][0] * vec.x() + m[1][1] * vec.y() + m[1][2] * vec.z();
        double z = m[2][0] * vec.x() + m[2][1] * vec.y() + m[2][2] * vec.z();
        return Vector3(x, y, z);
    }

    // Transform a normal (requires inverse transpose for non-uniform scaling)
    // For now, assumes uniform scaling - proper implementation would need inverse transpose
    Vector3 TransformNormal(const Vector3 &normal) const
    {
        Vector3 transformed = TransformVector(normal);
        return UnitVector(transformed);
    }

    // Matrix composition
    Transform operator*(const Transform &other) const
    {
        std::array<std::array<double, 4>, 4> result;
        for (int i = 0; i < 4; ++i)
        {
            for (int j = 0; j < 4; ++j)
            {
                result[i][j] = 0.0;
                for (int k = 0; k < 4; ++k)
                {
                    result[i][j] += m[i][k] * other.m[k][j];
                }
            }
        }
        return Transform(result);
    }

    // Get the inverse transform (simplified - assumes only translation, rotation, uniform scale)
    Transform Inverse() const
    {
        // For a proper implementation, you'd compute the full matrix inverse
        // This is a simplified version for common cases

        // Extract translation
        double tx = m[0][3];
        double ty = m[1][3];
        double tz = m[2][3];

        // Create inverse matrix
        std::array<std::array<double, 4>, 4> inv_matrix = {{{{m[0][0], m[1][0], m[2][0], -(m[0][0] * tx + m[1][0] * ty + m[2][0] * tz)}},
                                                            {{m[0][1], m[1][1], m[2][1], -(m[0][1] * tx + m[1][1] * ty + m[2][1] * tz)}},
                                                            {{m[0][2], m[1][2], m[2][2], -(m[0][2] * tx + m[1][2] * ty + m[2][2] * tz)}},
                                                            {{0.0, 0.0, 0.0, 1.0}}}};

        return Transform(inv_matrix);
    }

    Transform Translate(double x, double y, double z) const
    {
        return *this * Transform::FromTranslate(x, y, z);
    }

    Transform Translate(const Vector3 &offset) const
    {
        return *this * Transform::FromTranslate(offset);
    }

    Transform RotateRadY(double angle) const
    {
        return *this * Transform::FromRotateRadY(angle);
    }

    Transform RotateRadX(double angle) const
    {
        return *this * Transform::FromRotateRadX(angle);
    }

    Transform RotateRadZ(double angle) const
    {
        return *this * Transform::FromRotateZ(angle);
    }

    Transform RotateX(double degrees) const
    {
        return *this * Transform::FromRotateX(degrees);
    }

    Transform RotateY(double degrees) const
    {
        return *this * Transform::FromRotateY(degrees);
    }

    Transform RotateZ(double degrees) const
    {
        return *this * Transform::FromRotateZ(degrees);
    }

    Transform Scale(double s) const
    {
        return *this * Transform::FromScale(s);
    }

    // Static factory methods for common transformations
    static Transform FromTranslate(double x, double y, double z)
    {
        std::array<std::array<double, 4>, 4> matrix = {{{{1.0, 0.0, 0.0, x}},
                                                        {{0.0, 1.0, 0.0, y}},
                                                        {{0.0, 0.0, 1.0, z}},
                                                        {{0.0, 0.0, 0.0, 1.0}}}};
        return Transform(matrix);
    }

    static Transform FromTranslate(const Vector3 &offset)
    {
        return FromTranslate(offset.x(), offset.y(), offset.z());
    }

    // Uniform scaling
    // Note: Non-uniform scaling would require to fully implement
    // the inverse transpose for normal transformation to work correctly.
    static Transform FromScale(double s)
    {
        std::array<std::array<double, 4>, 4> matrix = {{{{s, 0.0, 0.0, 0.0}},
                                                        {{0.0, s, 0.0, 0.0}},
                                                        {{0.0, 0.0, s, 0.0}},
                                                        {{0.0, 0.0, 0.0, 1.0}}}};
        return Transform(matrix);
    }

    static Transform FromRotateRadX(double angle)
    {
        double c = std::cos(angle);
        double s = std::sin(angle);
        std::array<std::array<double, 4>, 4> matrix = {{{{1.0, 0.0, 0.0, 0.0}},
                                                        {{0.0, c, -s, 0.0}},
                                                        {{0.0, s, c, 0.0}},
                                                        {{0.0, 0.0, 0.0, 1.0}}}};
        return Transform(matrix);
    }

    static Transform FromRotateX(double angle)
    {
        return FromRotateRadX(angle * DegToRad);
    }

    static Transform FromRotateRadY(double angle)
    {
        double c = std::cos(angle);
        double s = std::sin(angle);
        std::array<std::array<double, 4>, 4> matrix = {{{{c, 0.0, s, 0.0}},
                                                        {{0.0, 1.0, 0.0, 0.0}},
                                                        {{-s, 0.0, c, 0.0}},
                                                        {{0.0, 0.0, 0.0, 1.0}}}};
        return Transform(matrix);
    }

    static Transform FromRotateY(double angle)
    {
        return FromRotateRadY(angle * DegToRad);
    }

    static Transform FromRotateRadZ(double angle)
    {
        double c = std::cos(angle);
        double s = std::sin(angle);
        std::array<std::array<double, 4>, 4> matrix = {{{{c, -s, 0.0, 0.0}},
                                                        {{s, c, 0.0, 0.0}},
                                                        {{0.0, 0.0, 1.0, 0.0}},
                                                        {{0.0, 0.0, 0.0, 1.0}}}};
        return Transform(matrix);
    }

    static Transform FromRotateZ(double angle)
    {
        return FromRotateRadZ(angle * DegToRad);
    }

    // Rotate around arbitrary axis
    static Transform FromRotateRad(double angle, const Vector3 &axis)
    {
        Vector3 a = UnitVector(axis);
        double c = std::cos(angle);
        double s = std::sin(angle);
        double t = 1.0 - c;

        std::array<std::array<double, 4>, 4> matrix = {{{{t * a.x() * a.x() + c, t * a.x() * a.y() - s * a.z(), t * a.x() * a.z() + s * a.y(), 0.0}},
                                                        {{t * a.x() * a.y() + s * a.z(), t * a.y() * a.y() + c, t * a.y() * a.z() - s * a.x(), 0.0}},
                                                        {{t * a.x() * a.z() - s * a.y(), t * a.y() * a.z() + s * a.x(), t * a.z() * a.z() + c, 0.0}},
                                                        {{0.0, 0.0, 0.0, 1.0}}}};
        return Transform(matrix);
    }

    static Transform FromRotate(double angle, const Vector3 &axis)
    {
        return FromRotateRad(angle * DegToRad, axis);
    }

    // Access matrix elements
    double operator()(int row, int col) const
    {
        return m[row][col];
    }

    double &operator()(int row, int col)
    {
        return m[row][col];
    }
};