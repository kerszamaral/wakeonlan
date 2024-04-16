#pragma once

#include "common/pcinfo.hpp"

namespace Subservices::Management::Exit
{
    void sender();

    void receiver(PC::atomic_pc_map_t &pc_map);

}