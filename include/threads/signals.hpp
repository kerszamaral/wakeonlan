#pragma once

#include <atomic>
#include <mutex>
#include <condition_variable>

namespace Threads
{
    class Signals
    {
    public:
        std::atomic<bool> is_manager;
        std::atomic<bool> run;
        std::atomic<bool> update;
        std::atomic<bool> manager_found;
        std::atomic<bool> ended;

        Signals(bool start_as_manager)
            : is_manager(start_as_manager),
              run(true),
              update(false),
              manager_found(false),
              ended(false)
        {
        }
    };
} // namespace Threads