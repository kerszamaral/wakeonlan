#pragma once

#include "common/pcinfo.hpp"
#include "threads/atomic.hpp"

void WriteCout(Threads::Atomic<pc_map_t> &pc_map);