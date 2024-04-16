#pragma once

#include "common/pcinfo.hpp"

namespace Subservices::Management::Wakeup
{
    void sender(PC::wakeups_queue &wakeups, PC::atomic_pc_map_t &pc_map);
}