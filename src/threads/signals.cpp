#include "threads/signals.hpp"

namespace Threads
{
    std::atomic_bool Signals::is_manager = false;
    std::atomic_bool Signals::run = true;
    std::atomic_bool Signals::update = false;
    std::atomic_uint32_t Signals::current_manager = 0;
    std::atomic_bool Signals::electing = false;
    std::atomic_uint32_t Signals::table_version = 0;
} // namespace Threads