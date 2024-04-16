#pragma once

#include "common/pcinfo.hpp"

namespace Subservices::Management::Update
{
    void update_pc_map(PC::new_pcs_queue &new_pcs, PC::atomic_pc_map_t &pc_map);
}