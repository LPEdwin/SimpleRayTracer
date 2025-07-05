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
    Vector3 TransformDirection(const Vector3 &vec) const
    {
        double x = m[0][0] * vec.x() + m[0][1] * vec.y() + m[0][2] * vec.z();
        double y = m[1][0] * vec.x() + m[1][1] * vec.y() + m[1][2] * vec.z();
        double z = m[2][0] * vec.x() + m[2][1] * vec.y() + m[2][2] * vec.z();
        return Vector3(x, y, z);
    }

    Vector3 TransformNormal(const Vector3 &normal) const
    {
        // Extract 3x3 linear part
        double a00 = m[0][0], a01 = m[0][1], a02 = m[0][2];
        double a10 = m[1][0], a11 = m[1][1], a12 = m[1][2];
        double a20 = m[2][0], a21 = m[2][1], a22 = m[2][2];

        // Compute inverse of the 3x3 matrix
        double det = a00 * (a11 * a22 - a12 * a21) - a01 * (a10 * a22 - a12 * a20) + a02 * (a10 * a21 - a11 * a20);

        if (std::abs(det) < 1e-8)
            throw std::runtime_error("TransformNormal: matrix is singular");

        double inv_det = 1.0 / det;

        // Compute inverse matrix (transpose as we go)
        double i00 = (a11 * a22 - a12 * a21) * inv_det;
        double i10 = -(a10 * a22 - a12 * a20) * inv_det;
        double i20 = (a10 * a21 - a11 * a20) * inv_det;

        double i01 = -(a01 * a22 - a02 * a21) * inv_det;
        double i11 = (a00 * a22 - a02 * a20) * inv_det;
        double i21 = -(a00 * a21 - a01 * a20) * inv_det;

        double i02 = (a01 * a12 - a02 * a11) * inv_det;
        double i12 = -(a00 * a12 - a02 * a10) * inv_det;
        double i22 = (a00 * a11 - a01 * a10) * inv_det;

        // Apply transpose of inverse
        Vector3 n;
        n[0] = i00 * normal.x() + i10 * normal.y() + i20 * normal.z();
        n[1] = i01 * normal.x() + i11 * normal.y() + i21 * normal.z();
        n[2] = i02 * normal.x() + i12 * normal.y() + i22 * normal.z();

        return UnitVector(n);
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

    Transform Inverse() const
    {
        constexpr double eps = std::numeric_limits<double>::epsilon();
        std::array<std::array<double, 4>, 4> inv{};

        // Extract upper-left 3x3 matrix (rotation + scale)
        double a00 = m[0][0], a01 = m[0][1], a02 = m[0][2];
        double a10 = m[1][0], a11 = m[1][1], a12 = m[1][2];
        double a20 = m[2][0], a21 = m[2][1], a22 = m[2][2];

        // Compute determinant of the 3x3 matrix
        double det = a00 * (a11 * a22 - a12 * a21) - a01 * (a10 * a22 - a12 * a20) + a02 * (a10 * a21 - a11 * a20);

        if (std::abs(det) < eps)
        {
            throw std::runtime_error("Transform::Inverse() - matrix is singular (non-invertible)");
        }

        double inv_det = 1.0 / det;

        // Compute inverse of upper-left 3x3 matrix (adjugate / determinant)
        inv[0][0] = (a11 * a22 - a12 * a21) * inv_det;
        inv[0][1] = -(a01 * a22 - a02 * a21) * inv_det;
        inv[0][2] = (a01 * a12 - a02 * a11) * inv_det;

        inv[1][0] = -(a10 * a22 - a12 * a20) * inv_det;
        inv[1][1] = (a00 * a22 - a02 * a20) * inv_det;
        inv[1][2] = -(a00 * a12 - a02 * a10) * inv_det;

        inv[2][0] = (a10 * a21 - a11 * a20) * inv_det;
        inv[2][1] = -(a00 * a21 - a01 * a20) * inv_det;
        inv[2][2] = (a00 * a11 - a01 * a10) * inv_det;

        // Invert the translation
        double tx = m[0][3];
        double ty = m[1][3];
        double tz = m[2][3];

        inv[0][3] = -(inv[0][0] * tx + inv[0][1] * ty + inv[0][2] * tz);
        inv[1][3] = -(inv[1][0] * tx + inv[1][1] * ty + inv[1][2] * tz);
        inv[2][3] = -(inv[2][0] * tx + inv[2][1] * ty + inv[2][2] * tz);

        // Last row stays the same
        inv[3][0] = inv[3][1] = inv[3][2] = 0.0;
        inv[3][3] = 1.0;

        return Transform(inv);
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

    Transform Scale(double sx, double sy, double sz) const
    {
        return *this * Transform::FromScale(sx, sy, sz);
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

    static Transform FromScale(double s)
    {
        return FromScale(s, s, s);
    }

    static Transform FromScale(double sx, double sy, double sz)
    {
        std::array<std::array<double, 4>, 4> matrix = {{{{sx, 0.0, 0.0, 0.0}},
                                                        {{0.0, sy, 0.0, 0.0}},
                                                        {{0.0, 0.0, sz, 0.0}},
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