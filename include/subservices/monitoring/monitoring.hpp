#pragma once

#include "common/pcinfo.hpp"

namespace Subservices::Monitoring
{
    void initialize(PC::atomic_pc_map_t &pc_map, PC::sleep_queue &sleep_status);
}