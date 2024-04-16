#include "threads/signals.hpp"

namespace Threads
{
    std::atomic<bool> Signals::is_manager = false;
    std::atomic<bool> Signals::run = true;
    std::atomic<bool> Signals::update = false;
    std::atomic<bool> Signals::manager_found = false;
} // namespace Threads