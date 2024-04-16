#include "subservices/management/update.hpp"

#include <thread>
#include "networking/sockets/udp.hpp"
#include "threads/signals.hpp"

namespace Subservices::Management::Update
{
    void update_pc_map(PC::new_pcs_queue &new_pcs, PC::atomic_pc_map_t &pc_map)
    {
        constexpr const auto CHECK_DELAY = std::chrono::milliseconds(100);
        while (Threads::Signals::run)
        {
            std::this_thread::sleep_for(CHECK_DELAY);
            auto maybe_new_pc = new_pcs.consume();

            if (maybe_new_pc.has_value())
            {
                const auto new_pc = maybe_new_pc.value();
                auto add_pc = [](PC::pc_map_t &pc_map, const PC::PCInfo &new_pc)
                {
                    pc_map.emplace(new_pc.get_hostname(), new_pc);
                };
                pc_map.execute(add_pc, new_pc);
                Threads::Signals::update = true;
            }
        }
    }
}