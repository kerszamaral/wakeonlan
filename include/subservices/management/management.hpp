#pragma once

#include "common/pcinfo.hpp"

namespace Subservices::Management
{
    void initialize(PC::new_pcs_queue &new_pcs, PC::atomic_pc_map_t &pc_map, PC::wakeups_queue &wakeups);
}