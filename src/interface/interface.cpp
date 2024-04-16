#include "interface/interface.hpp"
#include "interface/input.hpp"
#include "interface/output.hpp"
#include <thread>

void init_interface(PC::atomic_pc_map_t &pc_map, PC::wakeups_queue &wakeups)
{
    {
        std::vector<std::jthread> subservices;
        subservices.emplace_back(ReadCin, std::ref(wakeups));
        subservices.emplace_back(WriteCout, std::ref(pc_map));
    }
}