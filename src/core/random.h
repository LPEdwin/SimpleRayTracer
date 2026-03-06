#pragma once

#include <random>

inline std::mt19937 &GlobalRng()
{
    static thread_local std::mt19937 rng(std::random_device{}());
    return rng;
}

inline void SetGlobalRandomSeed(uint32_t seed)
{
    GlobalRng().seed(seed);
}

inline double RandomDouble()
{
    static thread_local std::uniform_real_distribution<double> dist(0.0, 1.0);
    return dist(GlobalRng());
}

inline double RandomDouble(double min, double max)
{
    return min + (max - min) * RandomDouble();
}