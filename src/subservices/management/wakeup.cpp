#include "subservices/management/wakeup.hpp"

#include <thread>
#include "networking/sockets/udp.hpp"
#include "threads/signals.hpp"

namespace Subservices::Management::Wakeup
{
    void wakeup_pc(PC::pc_map_t &pc_map, const PC::hostname_t &wakeup)
    {
        if (pc_map.contains(wakeup))
        {
            const auto &pc = pc_map.at(wakeup);
#ifdef DEBUG
            Networking::Sockets::UDP::broadcast_wakeup(pc.get_mac());
            std::cout << "Waking up " << wakeup << std::endl;
#else
            if (pc.get_status() == PC::STATUS::SLEEPING)
            {
                Networking::Sockets::UDP::broadcast_wakeup(pc.get_mac());
                std::cout << "Waking up " << wakeup << std::endl;
            }
            else
            {
                std::cout << wakeup << " is not sleeping" << std::endl;
            }
#endif
        }
        else
        {
            std::cout << "PC not found" << std::endl;
        }
    }

    void sender(PC::wakeups_queue &wakeups, PC::atomic_pc_map_t &pc_map)
    {
        while (Threads::Signals::run)
        {
            auto maybe_wakeup = wakeups.consume();
            if (maybe_wakeup.has_value())
            {
                const auto wakeup = maybe_wakeup.value();
                pc_map.execute(wakeup_pc, wakeup);
            }
        }
    }
}