#pragma once

#include <atomic>

/*
This subservice discovers new PCs on the network
and adds then them to the queue to be added to the pc_map
*/

void init_discovery(std::atomic<bool> &is_manager, std::atomic<bool> &run, std::atomic<bool> &update, std::atomic<bool> &manager_found);