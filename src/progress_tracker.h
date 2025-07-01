#pragma once

#define FMT_HEADER_ONLY
#include "fmt/core.h"
#include "fmt/chrono.h"
#include "fmt/format.h"
#include "fmt/printf.h"

#include <atomic>
#include <chrono>

class ProgressTracker
{
private:
    std::atomic<int> completed_lines{0};
    int total_lines;
    std::chrono::steady_clock::time_point start_time;
    mutable std::chrono::steady_clock::time_point last_update;

public:
    ProgressTracker(int total) : total_lines(total), start_time(std::chrono::steady_clock::now())
    {
        last_update = start_time;
    }

    void IncrementLine()
    {
        int current = ++completed_lines;

        if (current % 100 == 0 || current == total_lines)
        {
            ShowProgress(current, total_lines);
        }
    }

    void ShowProgress(int current, int total) const
    {
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - start_time).count();

        // Only update display every 100ms to avoid spam
        if (now - last_update < std::chrono::milliseconds(10) && current != total)
        {
            return;
        }
        last_update = now;

        double percentage = (double)current / total * 100.0;
        double lines_per_sec = elapsed > 0 ? (double)current / elapsed : 0;

        // Calculate ETA
        double eta_seconds = lines_per_sec > 0 ? (total - current) / lines_per_sec : 0;

        fmt::print(stderr, "\rProgress: {}/{} ({:.1f}%) - {:.1f} lines/sec - ETA: {:.0f}s",
                   current, total, percentage, lines_per_sec, eta_seconds);

        if (current == total)
        {
            fmt::print(stderr, "\nRendering completed in {}s\n", elapsed);
        }
    }
};

// #pragma once

// #include <iostream>
// #include <iomanip>

// void ShowProgress(int current, int total)
// {
//     static bool first_call = true;
//     if (first_call)
//     {
//         std::cout << "\033[?25l"; // Hide cursor
//         first_call = false;
//     }

//     int percent = static_cast<int>(100.0 * current / total);
//     std::cout << "\rProgress: " << std::setw(3) << percent << "% completed" << std::flush;

//     if (current == total)
//     {
//         std::cout << "\033[?25h" << std::endl; // Show cursor and newline
//     }
// }