#pragma once

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

#ifdef __GNUC__
#include <tbb/tbb.h>
#include <tbb/global_control.h>
#else
#include <ppl.h>
#endif

#include "camera.h"
#include "sphere.h"
#include "image.h"
#include "ray.h"
#include "vector3.h"
#include "hittable.h"
#include "progress.h"

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

void RenderLine(Image &image, int samplesPerPixel, const Camera &camera, const Vector3 &pixelDelta, int y, const Hittable &world)
{
    thread_local static bool printed = false;
    if (!printed)
    {
        string tmp = fmt::format(" processing line {}\n", y);
        std::cerr << "Thread " << std::this_thread::get_id() << tmp;
        printed = true;
    }

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

void Render(const Camera &camera, const Hittable &world, Image &image, int samplesPerPixel = 10)
{
    auto parallelismLimit = 0;
    auto parallelismMax = std::thread::hardware_concurrency();

#ifdef __GNUC__
    if (parallelismLimit > 0)
        tbb::global_control control(tbb::global_control::max_allowed_parallelism, parallelismLimit);
#else
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
#endif

    fmt::println("Hardware concurrency: {}/{}", parallelismLimit == 0 ? parallelismMax : parallelismLimit, parallelismMax);

    const Vector3 pixelDelta = Vector3(1.0f / image.width, 1.0f / image.height, 0.0f);

#ifdef __GNUC__
    // Use TBB parallel_for with GCC
    tbb::parallel_for(0, image.height, [&](int y)
                      { RenderLine(image, samplesPerPixel, camera, pixelDelta, y, world); });
#else
    // MSVC version using PPL's parallel_for
    Concurrency::parallel_for(0, image.height, [&](int y)
                              { RenderLine(image, samplesPerPixel, camera, pixelDelta, y, world); });

    // Clean up custom scheduler if used
    if (customScheduler)
    {
        concurrency::CurrentScheduler::Detach();
        customScheduler->Release();
    }
#endif

    // ShowProgress(y + 1, image.height);
}