#pragma once

#include "common/pcinfo.hpp"
#include "threads/atomic.hpp"
#include "threads/signals.hpp"

void init_interface(Threads::Atomic<pc_map_t> &pc_map, Threads::Signals &signals);