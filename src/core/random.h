#pragma once

#include <random>

double RandomDouble()
{
    static thread_local std::mt19937 rng(std::random_device{}());
    static thread_local std::uniform_real_distribution<double> dist(0.0, 1.0);
    return dist(rng);
}

inline double RandomDouble(double min, double max)
{
    return min + (max - min) * RandomDouble();
}