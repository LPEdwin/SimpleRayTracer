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
#include <stdexcept>

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

constexpr double Infinity = std::numeric_limits<double>::infinity();

struct PathState
{
    int DiffuseDepth;
    int SpecularDepth;

    int TotalDepth() const
    {
        return DiffuseDepth + SpecularDepth;
    }
};

class Renderer
{
public:
    int MaxDepth = 50;
    int SamplesPerPixelSqrt = 10;
    int ShadowRays = 2;
    int DiffuseSamples = 1;
    int MaxDiffuseDepth = 50;
    int SpecularSamples = 1;
    int MaxSpecularDepth = 50;
    unsigned int MaxThreadCount = 0;
    shared_ptr<EnvironmentMap> EnvironmentMap = nullptr;

private:
    Color GetColor(
        const Ray &ray,
        const Hittable &world,
        const vector<Quad *> &lights,
        PathState currentDepth) const
    {
        if (currentDepth.DiffuseDepth >= MaxDiffuseDepth ||
            currentDepth.SpecularDepth >= MaxSpecularDepth ||
            currentDepth.TotalDepth() >= MaxDepth)
            return Color(0, 0, 0);

        HitResult hit;
        if (!world.Hit(ray, hit, 0.001, Infinity))
            return EnvironmentMap ? EnvironmentMap->GetColor(ray) : Color(0, 0, 0);

        auto emission = hit.material->Emitted(hit, 0, 0);
        auto specular = Color(0, 0, 0);
        auto direct = Color(0, 0, 0);
        auto indirect = Color(0, 0, 0);

        ScatterResult scatterResult;
        if (!hit.material->Scatter(ray, hit, scatterResult))
            return emission;

        auto att = scatterResult.Attenuation;

        if (scatterResult.IsDeltaDistribution)
        {
            // Specular path
            auto ray_out = scatterResult.RayOut;
            auto nextDepth = currentDepth;
            nextDepth.SpecularDepth++;
            auto samples = currentDepth.TotalDepth() == 0 ? SpecularSamples : 1;
            for (int i = 0; i < samples; ++i)
            {
                specular += (att * GetColor(ray_out, world, lights, nextDepth));
            }
            specular /= samples;
        }
        else
        {
            // Direct Light
            //   aka Shadow Rays
            //   aka Light Sampling
            //   aka Next Event Estimation (NEE)

            const int n_light = currentDepth.TotalDepth() == 0 ? ShadowRays : 1;
            const int n_diffuse = currentDepth.TotalDepth() == 0 ? DiffuseSamples : 1;
            const auto lightSampler = CreateLightSampler(lights, hit.point);

            for (int i = 0; i < n_light; ++i)
            {
                auto lightDir = lightSampler->GenerateDirection();
                auto p_light = lightSampler->PdfValue(lightDir);
                auto p_bsdf = scatterResult.Sampler->PdfValue(lightDir);
                auto w = PowerHeuristic(n_light, p_light, n_diffuse, p_bsdf);

                auto lightRay = Ray(hit.point, lightDir);

                Color lightEmission;
                if (HitLight(lightRay, world, lights, Infinity, lightEmission))
                {
                    auto f_value = hit.material->Evaluate(ray, hit, lightRay);
                    direct += w * (att * f_value * lightEmission) / (n_light * p_light);
                }
            }

            // Diffuse path
            int retries = 0;
            for (int i = 0; i < n_diffuse; ++i)
            {
                auto scatterDir = scatterResult.Sampler->GenerateDirection();
                auto scatterRay = Ray(hit.point, scatterDir);
                if (!scatterDir.NearZero())
                {
                    auto p_light = lightSampler->PdfValue(scatterDir);
                    auto p_bsdf = scatterResult.Sampler->PdfValue(scatterDir);
                    auto f_value = hit.material->Evaluate(ray, hit, scatterRay);
                    auto nextDepth = currentDepth;
                    nextDepth.DiffuseDepth++;
                    Color lightEmission;
                    if (!HitLight(scatterRay, world, lights, Infinity, lightEmission))
                    {
                        // If no light was accidentally hit no MIS weights are needed.
                        indirect += (att * f_value * GetColor(scatterRay, world, lights, nextDepth)) / (n_diffuse * p_bsdf);
                    }
                    else
                    {
                        auto w = PowerHeuristic(n_diffuse, p_bsdf, n_light, p_light);
                        // The MIS weight w only applies to the direct light component of GetColor(...).
                        // Using a trick by subtracting the directLight first and adding the w weigthed directLight again.
                        // split = w*directLight + GetColor(...) - directLight
                        // TODO: Actually breaks if the direct light calculation isn't deterministic e.g. if using random fluctuations.
                        auto split = (w - 1) * lightEmission + GetColor(scatterRay, world, lights, nextDepth);
                        indirect += (att * f_value * split) / (n_diffuse * p_bsdf);
                    }
                }
                else if (retries++ < 10)
                {
                    i--;
                }
            }
        }
        return emission + specular + direct + indirect;
    }

    bool HitLight(const Ray &ray, const Hittable &world, const vector<Quad *> &lights, double maxDistance, Color &emission) const
    {
        HitResult hit;
        if (!world.Hit(ray, hit, 0.001, maxDistance))
        {
            return false;
        }

        for (const auto &l : lights)
        {
            if (hit.Object == l)
            {
                emission = hit.material->Emitted(hit, 0, 0);
                return true;
            }
        }
        return false;
    }

    double PowerHeuristic(int nA, double pA, int nB, double pB) const
    {
        double a = nA * pA;
        double b = nB * pB;
        double denom = (a * a + b * b);
        return denom > 0.0 ? (a * a) / denom : 0.0;
    }

    // Renders the scanline stratefied.
    void RenderLine(Image &image,
                    const Camera &camera,
                    const Hittable &world,
                    const vector<Quad *> &lights,
                    int line_number,
                    const Vector3 &pixelDelta)
    {
        const double pixelWidth = pixelDelta.x();
        const double pixelHeight = pixelDelta.y();
        const double subPixelWidth = pixelWidth / SamplesPerPixelSqrt;
        const double subPixelHeight = pixelHeight / SamplesPerPixelSqrt;
        const double baseV = line_number * pixelHeight;

        for (int w = 0; w < image.width; ++w)
        {
            Color color(0, 0, 0);
            const double baseU = w * pixelWidth;

            for (int i = 0; i < SamplesPerPixelSqrt; ++i)
            {
                for (int j = 0; j < SamplesPerPixelSqrt; ++j)
                {
                    // jitter
                    auto jx = RandomDouble();
                    auto jy = RandomDouble();

                    auto u = baseU + (i + jx) * subPixelWidth;
                    auto v = baseV + (j + jy) * subPixelHeight;

                    Ray ray = camera.GetRay(u, v);
                    color += GetColor(ray, world, lights, PathState());
                }
            }
            image.pixels[line_number][w] = color / (SamplesPerPixelSqrt * SamplesPerPixelSqrt);
        }
    }

    void RenderScanlineJittered(Image &image,
                                const Camera &camera,
                                const Hittable &world,
                                const vector<Quad *> &lights,
                                int line_number,
                                const Vector3 &pixelDelta)
    {
        const int N = SamplesPerPixelSqrt * SamplesPerPixelSqrt;
        const double pixelWidth = pixelDelta.x();
        const double pixelHeight = pixelDelta.y();

        for (int x = 0; x < image.width; ++x)
        {
            Color color(0, 0, 0);
            for (int s = 0; s < SamplesPerPixelSqrt; ++s)
            {
                // jitter
                auto jx = RandomDouble();
                auto jy = RandomDouble();

                Ray ray = camera.GetRay((x + jx) * pixelWidth,
                                        (line_number + jy) * pixelHeight);
                color += GetColor(ray, world, lights, PathState());
            }
            image.pixels[line_number][x] = color / SamplesPerPixelSqrt;
        }
    }

public:
    void Render(Image &image,
                const Camera &camera,
                const Hittable &world,
                const vector<Quad *> &lights)
    {
        if (lights.empty())
        {
            fmt::print(fg(fmt::color::orange),
                       "Warning: No lights registered for this scene.\n");
            if (ShadowRays > 0)
            {
                ShadowRays = 0;
                fmt::print(fg(fmt::color::orange),
                           "Warning: Set shadow ray count to 0.\n");
            }
        }

        auto hardwareLimit = std::thread::hardware_concurrency();
        auto threadCount = MaxThreadCount == 0 ? hardwareLimit : std::min(MaxThreadCount, hardwareLimit);
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