#include "framework/DataTypes.h"
#include <chrono>
#include <sstream>
#include <iomanip>

namespace devescape {

void GameState::addEvent(const std::string& event) {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);

    std::ostringstream oss;
    std::tm tm;
#ifdef _WIN32
    localtime_s(&tm, &time_t);
#else
    localtime_r(&time_t, &tm);
#endif
    oss << std::put_time(&tm, "%Y-%m-%dT%H:%M:%SZ") << ": " << event;
    eventLog.push_back(oss.str());
}

} // namespace devescape
