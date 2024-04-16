#pragma once

#include "common/pcinfo.hpp"
#include "threads/prodcosum.hpp"

void ReadCin(Threads::ProdCosum<hostname_t> &wakeups);