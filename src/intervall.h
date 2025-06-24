#pragma once

#include <limits>

struct Intervall
{
    double min;
    double max;

    Intervall() : min(-std::numeric_limits<double>::infinity()), max(std::numeric_limits<double>::infinity()) {}
    Intervall(double min, double max) : min(min), max(max) {}

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

    bool Overlaps(const Intervall &other) const
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

    static const Intervall kEmpty, kUnbounded;
};

const Intervall Intervall::kEmpty = Intervall(+std::numeric_limits<double>::infinity(), -std::numeric_limits<double>::infinity());
const Intervall Intervall::kUnbounded = Intervall(-std::numeric_limits<double>::infinity(), +std::numeric_limits<double>::infinity());