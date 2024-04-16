#include "threads/signals.hpp"

namespace Threads
{
    std::atomic_bool Signals::is_manager = false;
    std::atomic_bool Signals::run = true;
    std::atomic_bool Signals::update = false;
    std::atomic_bool Signals::manager_found = false;
} // namespace Threads