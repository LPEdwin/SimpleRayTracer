#pragma once

#include <limits>

struct Interval
{
    double min;
    double max;

    Interval() : min(-std::numeric_limits<double>::infinity()), max(std::numeric_limits<double>::infinity()) {}
    Interval(double min, double max) : min(min), max(max) {}

    Interval(const Interval &a, const Interval &b)
    {
        // Create the interval tightly enclosing the two input intervals.
        min = a.min <= b.min ? a.min : b.min;
        max = a.max >= b.max ? a.max : b.max;
    }

    // [min, max] - closed interval
    bool Contains(double value) const
    {
        return value >= min && value <= max;
    }

    // (min, max) - open interval
    bool Inside(double value) const
    {
        return value > min && value < max;
    }

    bool Overlaps(const Interval &other) const
    {
        return !(other.min > max || other.max < min);
    }

    bool IsEmpty() const
    {
        return min >= max;
    }

    double Length() const
    {
        return IsEmpty() ? 0.0 : max - min;
    }

    double Clamp(double x) const
    {
        if (x < min)
            return min;
        if (x > max)
            return max;
        return x;
    }

    Interval Expanded(double delta) const
    {
        auto padding = delta / 2;
        return Interval(min - padding, max + padding);
    }

    static const Interval kEmpty, kUnbounded;
};

const Interval Interval::kEmpty = Interval(+std::numeric_limits<double>::infinity(), -std::numeric_limits<double>::infinity());
const Interval Interval::kUnbounded = Interval(-std::numeric_limits<double>::infinity(), +std::numeric_limits<double>::infinity());

Interval operator+(const Interval &ival, double displacement)
{
    return Interval(ival.min + displacement, ival.max + displacement);
}

Interval operator+(double displacement, const Interval &ival)
{
    return ival + displacement;
}