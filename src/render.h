#pragma once

#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif

#define FMT_HEADER_ONLY
#include "fmt/core.h"
#include "fmt/chrono.h"
#include "fmt/format.h"
#include "fmt/printf.h"

#include <limits>
#include <execution>
#include <algorithm>
#include <iostream>
#include <ranges>
#include <thread>

#ifdef PPL
#include <ppl.h>
#else
#include <tbb/tbb.h>
#include <tbb/global_control.h>
#endif

#include "camera.h"
#include "sphere.h"
#include "image.h"
#include "ray.h"
#include "vector3.h"
#include "hittable.h"
#include "progress_tracker.h"

static const double inf = std::numeric_limits<double>::infinity();

Color GetColor(const Ray &ray, const Hittable &world, int depth = 50)
{
    if (depth <= 0)
        return Color(0, 0, 0);

    HitResult hit{};
    if (world.Hit(ray, hit, 0.001, inf))
    {
        Color attenuation;
        Ray r2;
        if (hit.material->Scatter(ray, hit, attenuation, r2))
            return attenuation * GetColor(r2, world, depth - 1);

        // If the ray does not scatter, return black.
        return Color(0, 0, 0);
    }

    auto dir = UnitVector(ray.direction);
    auto a = 0.5 * (dir.y() + 1.0);
    return (1.0 - a) * Vector3(1.0, 1.0, 1.0) + a * Vector3(0.5, 0.7, 1.0);
}

void RenderLine(Image &image,
                int samplesPerPixel,
                const Camera &camera,
                const Vector3 &pixelDelta,
                int y,
                const Hittable &world)
{
    for (int x = 0; x < image.width; ++x)
    {
        Color color(0, 0, 0);
        for (int s = 0; s < samplesPerPixel; ++s)
        {
            auto aaOffset = Vector3(RandomDouble() - 0.5, RandomDouble() - 0.5, 0.0);
            Ray ray = camera.GetRay((x + aaOffset.x()) * pixelDelta.x(),
                                    (y + aaOffset.y()) * pixelDelta.y());
            color += GetColor(ray, world);
        }
        image.pixels[y][x] = color / samplesPerPixel;
    }
}

void Render(const Camera &camera,
            const Hittable &world,
            Image &image,
            int samplesPerPixel = 100)
{
    auto parallelismLimit = 0;
    auto parallelismMax = std::thread::hardware_concurrency();
    ProgressTracker progressTracker(image.height);

#ifdef PPL
    Concurrency::Scheduler *customScheduler = nullptr;
    if (parallelismLimit > 0)
    {
        // Create custom scheduler with concurrency limit
        Concurrency::SchedulerPolicy policy;
        policy.SetConcurrencyLimits(parallelismLimit, parallelismLimit);
        customScheduler = Concurrency::Scheduler::Create(policy);

        // Attach custom scheduler to current context
        customScheduler->Attach();
    }
#else
    if (parallelismLimit > 0)
        tbb::global_control control(tbb::global_control::max_allowed_parallelism, parallelismLimit);
#endif

    fmt::println("Hardware concurrency: {}/{}", parallelismLimit == 0 ? parallelismMax : parallelismLimit, parallelismMax);

    const Vector3 pixelDelta = Vector3(1.0f / image.width, 1.0f / image.height, 0.0f);

#if defined(PPL) && defined(_MSC_VER)
    // MSVC version using PPL's parallel_for
    Concurrency::parallel_for(0, image.height, [&](int y)
                              { RenderLine(image, samplesPerPixel, camera, pixelDelta, y, world);
                                progressTracker.IncrementLine(); });

    if (customScheduler)
    {
        concurrency::CurrentScheduler::Detach();
        customScheduler->Release();
    }
#else
    // Use TBB parallel_for as default
    tbb::parallel_for(0, image.height, [&](int y)
                      { RenderLine(image, samplesPerPixel, camera, pixelDelta, y, world); 
                        progressTracker.IncrementLine(); });
#endif
}