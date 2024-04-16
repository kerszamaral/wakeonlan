#include "subservices/interface/interface.hpp"

#include <vector>
#include <thread>

#include "subservices/interface/input.hpp"
#include "subservices/interface/output.hpp"

namespace Subservices::Interface
{
    void initialize(PC::atomic_pc_map_t &pc_map, PC::wakeups_queue &wakeups)
    {
        {
            std::vector<std::jthread> subservices;
            subservices.emplace_back(Input::ReadCin, std::ref(wakeups));
            subservices.emplace_back(Output::WriteCout, std::ref(pc_map));
        }
    }
}