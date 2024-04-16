#pragma once

#include <atomic>

namespace Threads
{
    class Signals
    {
    public:
        static std::atomic<bool> is_manager;
        static std::atomic<bool> run;
        static std::atomic<bool> update;
        static std::atomic<bool> manager_found;
    };
} // namespace Threads