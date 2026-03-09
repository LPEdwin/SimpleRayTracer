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

inline std::mt19937 &FixedRng()
{
    static thread_local std::mt19937 rng(1337);
    return rng;
}

inline double FixedRandomDouble()
{
    static thread_local std::uniform_real_distribution<double> dist(0.0, 1.0);
    return dist(FixedRng());
}

inline double FixedRandomDouble(double min, double max)
{
    return min + (max - min) * FixedRandomDouble();
}