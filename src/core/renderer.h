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

#include "core/camera.h"
#include "collision/sphere.h"
#include "io/image.h"
#include "core/ray.h"
#include "core/vector3.h"
#include "core/hittable.h"
#include "io/progress_tracker.h"
#include "core/environment_map.h"

class Renderer
{
public:
    int maxDepth = 50;
    int samplesPerPixel = 100;
    unsigned int maxThreadCount = 0;
    shared_ptr<EnvironmentMap> environmentMap = nullptr;

private:
    Color GetColor(const Ray &ray, const Hittable &world, int currentDepth) const
    {
        constexpr double inf = std::numeric_limits<double>::infinity();

        if (currentDepth <= 0)
            return Color(0, 0, 0);

        HitResult hit{};
        if (world.Hit(ray, hit, 0.001, inf))
        {
            Color attenuation;
            Ray secondaryRay;
            if (hit.material->Scatter(ray, hit, attenuation, secondaryRay))
                return attenuation * GetColor(secondaryRay, world, currentDepth - 1) + hit.material->Emitted(hit.point, 0, 0);

            return hit.material->Emitted(hit.point, 0, 0);
        }
        return environmentMap ? environmentMap->GetColor(ray) : Color(0, 0, 0);
    }

    void RenderLine(Image &image,
                    const Camera &camera,
                    const Hittable &world,
                    int line_number,
                    const Vector3 &pixelDelta)
    {
        for (int x = 0; x < image.width; ++x)
        {
            Color color(0, 0, 0);
            for (int s = 0; s < samplesPerPixel; ++s)
            {
                auto sampleOffset = Vector3(RandomDouble() - 0.5, RandomDouble() - 0.5, 0.0);
                Ray ray = camera.GetRay((x + sampleOffset.x()) * pixelDelta.x(),
                                        (line_number + sampleOffset.y()) * pixelDelta.y());
                color += GetColor(ray, world, maxDepth);
            }
            image.pixels[line_number][x] = color / samplesPerPixel;
        }
    }

public:
    void Render(Image &image,
                const Camera &camera,
                const Hittable &world)
    {
        auto hardwareLimit = std::thread::hardware_concurrency();
        auto threadCount = maxThreadCount = 0 ? 0 : std::min(maxThreadCount, hardwareLimit);
        ProgressTracker progressTracker(image.height);

#ifdef PPL
        Concurrency::Scheduler *customScheduler = nullptr;
        if (threadCount > 0)
        {
            // Create custom scheduler with concurrency limit
            Concurrency::SchedulerPolicy policy;
            policy.SetConcurrencyLimits(threadCount, threadCount);
            customScheduler = Concurrency::Scheduler::Create(policy);

            // Attach custom scheduler to current context
            customScheduler->Attach();
        }
#else
        // needs to stay in scope until TBB parallel_for is done
        std::unique_ptr<tbb::global_control> control;
        if (threadCount > 0)
            control = std::make_unique<tbb::global_control>(tbb::global_control::max_allowed_parallelism, threadCount);
#endif

        fmt::println("Hardware concurrency: {}/{}", threadCount == 0 ? hardwareLimit : threadCount, hardwareLimit);

        const Vector3 pixelDelta = Vector3(1.0f / image.width, 1.0f / image.height, 0.0f);

#if defined(PPL) && defined(_MSC_VER)
        // MSVC version using PPL's parallel_for
        Concurrency::parallel_for(0, image.height, [&](int y)
                                  { RenderLine(image, camera, world, y, pixelDelta);
                                progressTracker.IncrementLine(); });

        if (customScheduler)
        {
            concurrency::CurrentScheduler::Detach();
            customScheduler->Release();
        }
#else
        // Use TBB parallel_for as default
        tbb::parallel_for(0, image.height, [&](int y)
                          { RenderLine(image, camera, world, y, pixelDelta); 
                        progressTracker.IncrementLine(); });
#endif
    }
};