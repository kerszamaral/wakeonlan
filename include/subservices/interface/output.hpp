#pragma once

#include "common/pcinfo.hpp"

namespace Subservices::Interface::Output
{
    void WriteCout(PC::atomic_pc_map_t &pc_map);
}