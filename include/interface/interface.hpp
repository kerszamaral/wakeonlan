#pragma once

#include "common/pcinfo.hpp"
#include "threads/atomic.hpp"
#include "threads/prodcosum.hpp"

void init_interface(Threads::Atomic<pc_map_t> &pc_map, Threads::ProdCosum<hostname_t> &wakeups);