#include "subservices/management/management.hpp"

#include <vector>
#include <thread>

#include "threads/signals.hpp"
#include "subservices/management/update.hpp"
#include "subservices/management/exit.hpp"
#include "subservices/management/wakeup.hpp"

namespace Subservices::Management
{
    void initialize(PC::new_pcs_queue &new_pcs, PC::atomic_pc_map_t &pc_map, PC::wakeups_queue &wakeups, PC::sleep_queue &sleep_status)
    {
        {
            std::vector<std::jthread> subservices;
            subservices.emplace_back(Update::update_pc_map, std::ref(new_pcs), std::ref(pc_map));
            subservices.emplace_back(Update::update_sleep_status, std::ref(sleep_status), std::ref(pc_map));
            subservices.emplace_back(Wakeup::sender, std::ref(wakeups), std::ref(pc_map));
            subservices.emplace_back(Exit::receiver, std::ref(pc_map));
            subservices.emplace_back(Exit::sender);
#ifdef DEBUG
            subservices.emplace_back(Exit::transition);
#endif
        }
    }
}