#pragma once

#include "common/pcinfo.hpp"

/*
This subservice discovers new PCs on the network
and adds then them to the queue to be added to the pc_map
*/

void init_discovery(PC::new_pcs_queue &new_pcs);