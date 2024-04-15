#include "interface/interface.hpp"
#include "interface/input.hpp"
#include "interface/output.hpp"
#include <thread>

void init_interface(Threads::Atomic<pc_map_t> &pc_map, Threads::Signals &signals, Threads::ProdCosum<hostname_t> &wakeups)
{
    {
        std::vector<std::jthread> subservices;
        subservices.emplace_back(ReadCin, std::ref(signals), std::ref(wakeups));
        subservices.emplace_back(WriteCout, std::ref(pc_map), std::ref(signals));
    }
}