#pragma once

#include "common/pcinfo.hpp"
#include "threads/prodcosum.hpp"
#include "threads/atomic.hpp"
#include "threads/signals.hpp"

void init_management(Threads::ProdCosum<PCInfo> &new_pcs, Threads::Atomic<pc_map_t> &pc_map, Threads::Signals &signals);