#include "subservices/management/update.hpp"

#include <thread>
#include "networking/sockets/udp.hpp"
#include "threads/signals.hpp"
#include "threads/delays.hpp"

namespace Subservices::Management::Update
{
    void add_pc(PC::pc_map_t &pc_map, const PC::PCInfo &new_pc, PC::updates_queue &updates)
    {
        //? If a pc rejoins the network, also we need to update the pc_map
        if (!Threads::Signals::is_manager)
        {
            pc_map.clear();
        }
        const auto &ret = pc_map.insert_or_assign(new_pc.get_hostname(), new_pc);
        if (ret.second)
        {
            Threads::Signals::update = true;
            Threads::Signals::update.notify_all();
            updates.produce(std::make_pair(PC::UPDATE_TYPE::ADD, new_pc));
        }
    };

    void update_pc_map(PC::new_pcs_queue &new_pcs, PC::atomic_pc_map_t &pc_map, PC::updates_queue &updates)
    {
        while (Threads::Signals::run)
        {
            std::this_thread::sleep_for(Threads::Delays::CHECK_DELAY);
            auto maybe_new_pc = new_pcs.consume();

            if (maybe_new_pc.has_value())
            {
                const auto new_pc = maybe_new_pc.value();
                pc_map.execute(add_pc, new_pc, updates);
            }
        }
    }

    void update_status(PC::pc_map_t &pc_map, const PC::hostname_t &hostname, PC::STATUS status, PC::updates_queue &updates)
    {
        if (pc_map.contains(hostname))
        {
            auto &pc_info = pc_map.at(hostname);
            if (pc_info.get_status() != status)
            {
                pc_info.set_status(status);
                Threads::Signals::update = true;
                Threads::Signals::update.notify_all();
                updates.produce(std::make_pair(PC::UPDATE_TYPE::CHANGE, pc_info));
            }
        }
    }

    void update_sleep_status(PC::sleep_queue &sleep_status, PC::atomic_pc_map_t &pc_map, PC::updates_queue &updates)
    {
        while (Threads::Signals::run)
        {
            std::this_thread::sleep_for(Threads::Delays::CHECK_DELAY);
            auto maybe_asleep = sleep_status.consume();

            if (maybe_asleep.has_value())
            {
                const auto [hostname, status] = maybe_asleep.value();
                pc_map.execute(update_status, hostname, status, updates);
            }
        }
    }
}