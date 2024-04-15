#pragma once

#include <functional>
#include "threads/signals.hpp"

namespace Shutdown
{
    void graceful_setup(Threads::Signals &signals);

    void graceful_shutdown();
}
