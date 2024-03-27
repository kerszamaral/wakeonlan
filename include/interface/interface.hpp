#pragma once

#include <atomic>
#include "common/pcinfo.hpp"

void init_interface(const pc_map_t &pc_map, std::atomic<bool> &run, std::atomic<bool> &update);