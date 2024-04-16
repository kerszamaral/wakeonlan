#include "subservices/management/wakeup.hpp"

#include <thread>
#include "networking/sockets/udp.hpp"
#include "threads/signals.hpp"

namespace Subservices::Management::Wakeup
{
    void sender(PC::wakeups_queue &wakeups, PC::atomic_pc_map_t &pc_map)
    {
        while (Threads::Signals::run)
        {
            auto maybe_wakeup = wakeups.consume();
            if (maybe_wakeup.has_value())
            {
                const auto wakeup = maybe_wakeup.value();
                auto wakeup_pc = [&wakeup](PC::pc_map_t &pc_map)
                {
                    if (pc_map.contains(wakeup))
                    {
                        const auto &pc = pc_map.at(wakeup);
                        if (pc.get_status() == PC::STATUS::SLEEPING)
                        {
                            Networking::Sockets::UDP::broadcast_wakeup(pc.get_mac());
                            std::cout << "Waking up " << wakeup << std::endl;
                        }
                        else
                        {
                            std::cout << wakeup << " is not sleeping" << std::endl;
                        }
                    }
                    else
                    {
                        std::cout << "PC not found" << std::endl;
                    }
                };
                pc_map.execute(wakeup_pc);
            }
        }
    }
}