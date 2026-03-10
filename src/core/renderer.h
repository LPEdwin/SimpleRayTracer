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
#include <fmt/color.h>

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
#include "collision/quad.h"
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
    int samplesPerPixelSqrt = 10;
    unsigned int maxThreadCount = 0;
    shared_ptr<EnvironmentMap> environmentMap = nullptr;

private:
    Color GetColor(
        const Ray &ray,
        const Hittable &world,
        const vector<shared_ptr<Quad>> &lights,
        int currentDepth) const
    {
        constexpr double inf = std::numeric_limits<double>::infinity();

        if (currentDepth <= 0)
            return Color(0, 0, 0);

        HitResult hit{};
        if (world.Hit(ray, hit, 0.001, inf))
        {
            ScatterResult scatter_result;

            if (hit.material->Scatter(ray, hit, scatter_result))
            {
                if (scatter_result.IsDeltaDistribution)
                {
                    auto att = scatter_result.Attenuation;
                    auto ray_out = scatter_result.RayOut;
                    return (att * GetColor(ray_out, world, lights, currentDepth - 1)) + hit.material->Emitted(hit, 0, 0);
                }
                else
                {
                    auto att = scatter_result.Attenuation;
                    std::vector<std::shared_ptr<SamplingPdf>> samplers{scatter_result.Sampler};
                    for (const auto &light : lights)
                    {
                        samplers.push_back(std::make_shared<HittableSampling>(*light, hit.point));
                    }
                    auto mixtureSampler = std::make_shared<MixtureSampling>(samplers);
                    auto scattered_ray = Ray(hit.point, mixtureSampler->GenerateDirection());
                    if (!scattered_ray.direction.NearZero())
                    {
                        auto f_value = hit.material->Evaluate(ray, hit, scattered_ray);
                        auto pdf = mixtureSampler->PdfValue(scattered_ray.direction);
                        return (att * f_value * GetColor(scattered_ray, world, lights, currentDepth - 1)) / pdf + hit.material->Emitted(hit, 0, 0);
                    }
                }
            }

            return hit.material->Emitted(hit, 0, 0);
        }
        return environmentMap ? environmentMap->GetColor(ray) : Color(0, 0, 0);
    }

    // Renders the scanline stratefied.
    void RenderLine(Image &image,
                    const Camera &camera,
                    const Hittable &world,
                    const vector<shared_ptr<Quad>> &lights,
                    int line_number,
                    const Vector3 &pixelDelta)
    {
        const double pixelWidth = pixelDelta.x();
        const double pixelHeight = pixelDelta.y();
        const double subPixelWidth = pixelWidth / samplesPerPixelSqrt;
        const double subPixelHeight = pixelHeight / samplesPerPixelSqrt;
        const double baseV = line_number * pixelHeight;

        for (int w = 0; w < image.width; ++w)
        {
            Color color(0, 0, 0);
            const double baseU = w * pixelWidth;

            for (int i = 0; i < samplesPerPixelSqrt; ++i)
            {
                for (int j = 0; j < samplesPerPixelSqrt; ++j)
                {
                    // jitter
                    auto jx = RandomDouble();
                    auto jy = RandomDouble();

                    auto u = baseU + (i + jx) * subPixelWidth;
                    auto v = baseV + (j + jy) * subPixelHeight;

                    Ray ray = camera.GetRay(u, v);
                    color += GetColor(ray, world, lights, maxDepth);
                }
            }
            image.pixels[line_number][w] = color / (samplesPerPixelSqrt * samplesPerPixelSqrt);
        }
    }

    void RenderScanlineJittered(Image &image,
                                const Camera &camera,
                                const Hittable &world,
                                const vector<shared_ptr<Quad>> &lights,
                                int line_number,
                                const Vector3 &pixelDelta)
    {
        const int N = samplesPerPixelSqrt * samplesPerPixelSqrt;
        const double pixelWidth = pixelDelta.x();
        const double pixelHeight = pixelDelta.y();

        for (int x = 0; x < image.width; ++x)
        {
            Color color(0, 0, 0);
            for (int s = 0; s < samplesPerPixelSqrt; ++s)
            {
                // jitter
                auto jx = RandomDouble();
                auto jy = RandomDouble();

                Ray ray = camera.GetRay((x + jx) * pixelWidth,
                                        (line_number + jy) * pixelHeight);
                color += GetColor(ray, world, lights, maxDepth);
            }
            image.pixels[line_number][x] = color / samplesPerPixelSqrt;
        }
    }

public:
    void Render(Image &image,
                const Camera &camera,
                const Hittable &world,
                const vector<shared_ptr<Quad>> &lights)
    {
        if (lights.empty())
            fmt::print(fg(fmt::color::orange),
                       "Warning: No lights registered for this scene.\n");

        auto hardwareLimit = std::thread::hardware_concurrency();
        auto threadCount = maxThreadCount == 0 ? hardwareLimit : std::min(maxThreadCount, hardwareLimit);
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
                                  { RenderLine(image, camera, world, lights, y, pixelDelta);
                                progressTracker.IncrementLine(); });

        if (customScheduler)
        {
            concurrency::CurrentScheduler::Detach();
            customScheduler->Release();
        }
#else
        // Use TBB parallel_for as default
        tbb::parallel_for(0, image.height, [&](int y)
                          { RenderLine(image, camera, world, lights, y, pixelDelta); 
                        progressTracker.IncrementLine(); });
#endif
    }
};