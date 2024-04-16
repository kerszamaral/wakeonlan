#pragma once

#include "common/pcinfo.hpp"

namespace Subservices::Interface
{
    void initialize(PC::atomic_pc_map_t &pc_map, PC::wakeups_queue &wakeups);
}