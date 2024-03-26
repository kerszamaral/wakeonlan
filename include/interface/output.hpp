#pragma once

#include <atomic>
#include "common/constants.hpp"

void WriteCout(const pc_map_t &pc_map, std::atomic<bool> &run, std::atomic<bool> &update);