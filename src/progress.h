#pragma once

#include <iostream>
#include <iomanip>

void ShowProgress(int current, int total)
{
    static bool first_call = true;
    if (first_call)
    {
        std::cout << "\033[?25l"; // Hide cursor
        first_call = false;
    }

    int percent = static_cast<int>(100.0 * current / total);
    std::cout << "\rProgress: " << std::setw(3) << percent << "% completed" << std::flush;

    if (current == total)
    {
        std::cout << "\033[?25h" << std::endl; // Show cursor and newline
    }
}