#include <chrono>
#include <iostream>
#include <iomanip>
#include <sstream>

template <typename Duration>
std::string formatDuration(Duration d)
{
    auto hours = std::chrono::duration_cast<std::chrono::hours>(d);
    auto minutes = std::chrono::duration_cast<std::chrono::minutes>(d % std::chrono::hours(1));
    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(d % std::chrono::minutes(1));

    std::ostringstream oss;
    oss << std::setfill('0') << std::setw(2) << hours.count() << ":"
        << std::setfill('0') << std::setw(2) << minutes.count() << ":"
        << std::setfill('0') << std::setw(2) << seconds.count();
    return oss.str();
}