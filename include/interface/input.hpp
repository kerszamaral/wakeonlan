#pragma once

#include "threads/signals.hpp"
#include "common/pcinfo.hpp"
#include "threads/prodcosum.hpp"

void ReadCin(Threads::Signals &signals, Threads::ProdCosum<hostname_t> &wakeups);