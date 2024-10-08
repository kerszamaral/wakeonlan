#pragma once

#include "common/pcinfo.hpp"

namespace Subservices::Replication
{
    void initialize(PC::atomic_pc_map_t &pc_map, PC::updates_queue &updates);
} // namespace Subservices::Replication