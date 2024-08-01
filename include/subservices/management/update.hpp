#pragma once

#include "common/pcinfo.hpp"

namespace Subservices::Management::Update
{
    void update_pc_map(PC::new_pcs_queue &new_pcs, PC::atomic_pc_map_t &pc_map, PC::updates_queue &updates);

    void update_sleep_status(PC::sleep_queue &sleep_status, PC::atomic_pc_map_t &pc_map, PC::updates_queue &updates);
}