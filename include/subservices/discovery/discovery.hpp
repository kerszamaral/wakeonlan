#pragma once

#include "common/pcinfo.hpp"

/*
This subservice discovers new PCs on the network
and adds then them to the queue to be added to the pc_map
*/

namespace Subservices::Discovery
{
    void initialize(PC::new_pcs_queue &new_pcs);
}