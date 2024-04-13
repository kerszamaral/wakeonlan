#pragma once

#include <atomic>
#include "common/pcinfo.hpp"
#include "threads/prodcosum.hpp"
#include "threads/signals.hpp"

/*
This subservice discovers new PCs on the network
and adds then them to the queue to be added to the pc_map
*/

void init_discovery(Threads::ProdCosum<PCInfo> &new_pcs, Threads::Signals &signals);