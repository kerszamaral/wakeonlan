#include "subservices/management/wakeup.hpp"

#include <thread>
#include "networking/sockets/udp.hpp"
#include "threads/signals.hpp"
#include "threads/delays.hpp"
#include <syncstream>

namespace Subservices::Management::Wakeup
{
    void wakeup_pc(PC::pc_map_t &pc_map, const PC::hostname_t &wakeup)
    {
        std::osyncstream tout(std::cout);
        if (pc_map.contains(wakeup))
        {
            const auto &pc = pc_map.at(wakeup);
            if (pc.get_status() == PC::STATUS::SLEEPING)
            {
                Networking::Sockets::UDP::broadcast_wakeup(pc.get_mac());
                tout << "Waking up " << wakeup << std::endl;
            }
            else
            {
                tout << wakeup << " is not sleeping" << std::endl;
            }
        }
        else
        {
            tout << "PC not found" << std::endl;
        }
    }

    void sender(PC::wakeups_queue &wakeups, PC::atomic_pc_map_t &pc_map)
    {
        while (Threads::Signals::run)
        {
            std::this_thread::sleep_for(Threads::Delays::CHECK_DELAY);
            auto maybe_wakeup = wakeups.consume();
            if (maybe_wakeup.has_value())
            {
                const auto wakeup = maybe_wakeup.value();
                pc_map.execute(wakeup_pc, wakeup);
            }
        }
    }
}