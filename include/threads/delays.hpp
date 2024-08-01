#pragma once

#include <chrono>

namespace Threads::Delays
{
    constexpr const auto FLUSH_DELAY = std::chrono::milliseconds(1);
    constexpr const auto INPUT_DELAY = std::chrono::milliseconds(50);
    constexpr const auto WAIT_DELAY = std::chrono::milliseconds(100);
    constexpr const auto CHECK_DELAY = std::chrono::milliseconds(100);
    constexpr const auto MANAGER_TIMEOUT = std::chrono::milliseconds(1000);
}