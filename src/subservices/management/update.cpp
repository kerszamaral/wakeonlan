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
                Threads::Signals::update.notify_all();
            }
        }
    }

    void update_sleep_status(PC::wakeups_queue &wakeups, PC::sleep_queue &sleep_status)
    {
        constexpr const auto CHECK_DELAY = std::chrono::milliseconds(100);
        // const auto update_pcs = [&local_pc_map](PC::pc_map_t &pc_map) -> void
        // {
        //     bool should_update = false;
        //     for (const auto &[hostname, ipv4, status] : local_pc_map)
        //     {
        //         if (pc_map.contains(hostname))
        //         {
        //             auto &pc_info = pc_map.at(hostname);
        //             if (pc_info.get_status() != status)
        //             {
        //                 pc_info.set_status(status);
        //                 should_update = true;
        //             }
        //         }
        //     }
        // };
        // while (Threads::Signals::run)
        // {
        //     std::this_thread::sleep_for(CHECK_DELAY);
        //     auto maybe_wakeup = wakeups.consume();

        //     if (maybe_wakeup.has_value())
        //     {
        //         const auto [hostname, status] = maybe_wakeup.value();
        //         auto update_status = [&hostname, status](PC::pc_map_t &pc_map)
        //         {
        //             pc_map[hostname].set_status(status);
        //         };
        //         sleep_status.execute(update_status);
        //     }
        // }
    }
}