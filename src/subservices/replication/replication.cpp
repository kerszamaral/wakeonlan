#include "subservices/replication/replication.hpp"

#include <thread>

#include "threads/delays.hpp"
#include "threads/signals.hpp"
#include "networking/sockets/udp.hpp"
#include "networking/packets/util.hpp"

namespace Subservices::Replication
{
    PC::pc_map_t deepcopy_pc_map(const PC::pc_map_t &pc_map)
    {
        PC::pc_map_t pc_map_copy;
        auto it = pc_map.begin();
        while (it != pc_map.end())
        {
            pc_map_copy.insert_or_assign(it->first, it->second);
            ++it;
        }
        return pc_map_copy;
    }

    void copy_backup_to_main(PC::pc_map_t &main, const PC::pc_map_t &backup, const PC::hostname_t &our_hostname)
    {
        main.clear();
        for (auto [hostname, pc] : backup)
        {
            if (pc.get_hostname() != our_hostname)
            {
                // We don't want to add ourselves to the map
                continue;
            }
            if (pc.get_is_manager())
            {
                pc.set_is_manager(false);
            }
            main.insert_or_assign(hostname, pc);
        }
        Threads::Signals::update = true;
        Threads::Signals::update.notify_all();
        Threads::Signals::replication_update = true;
        Threads::Signals::replication_update.notify_all();
    }

    void remove_everything_but_manager(PC::pc_map_t &pc_map)
    {
        opt::optional<PC::PCInfo> manager = opt::nullopt;
        for (auto pc : pc_map)
        {
            if (pc.second.get_is_manager())
            {
                manager = pc.second;
                break;
            }
        }
        pc_map.clear();
        if (manager.has_value())
        {
            pc_map.insert_or_assign(manager->get_hostname(), manager.value());
        }
    }

    void initialize(PC::atomic_pc_map_t &pc_map)
    {
        using namespace Networking;
        auto conn = Sockets::UDP(Addresses::REPLICATION_PORT);

        PC::pc_map_t backup_map = pc_map.execute(deepcopy_pc_map);
        bool was_manager = Threads::Signals::is_manager;

        // We need our pc info
        const auto our_hostname = PC::getHostname();
        const auto our_mac = Networking::Addresses::Mac::FromMachine();
        const auto our_ip = Networking::Addresses::IPv4::FromMachine();
        const auto our_status = PC::STATUS::AWAKE;
        const auto ourselves = PC::PCInfo(our_hostname, our_mac, our_ip, our_status, was_manager);
        backup_map.insert_or_assign(our_hostname, ourselves);

        while (Threads::Signals::run)
        {
            if (was_manager != Threads::Signals::is_manager)
            {
                was_manager = Threads::Signals::is_manager;
                if (was_manager)
                {
                    // We are now the manager
                    pc_map.execute(copy_backup_to_main, backup_map, our_hostname);
                }
                else
                {
                    // We are no longer the manager
                    pc_map.execute(remove_everything_but_manager);
                }
            }

            if (Threads::Signals::is_manager)
            {
                // Manager sends the table to all clients
                if (Threads::Signals::replication_update)
                {
                    Threads::Signals::table_version++;
                    const uint32_t table_version = Threads::Signals::table_version;
                    auto pc_map_copy = pc_map.execute(deepcopy_pc_map);
                    auto payload = std::make_pair(table_version, pc_map_copy);
                    auto packet = Packets::Packet(Packets::PacketType::SSREP, payload);
                    conn.send_broadcast(packet, Addresses::REPLICATION_PORT);
                    Threads::Signals::replication_update = false;
                }
            }
            else
            {
                // Client receives the table from the manager
                const auto maybe_packet = conn.wait_and_receive_packet(Threads::Delays::CHECK_DELAY);
                if (!maybe_packet.has_value())
                {
                    continue;
                }
                auto &[packet, addr] = maybe_packet.value();
                if (packet.getType() != Packets::PacketType::SSREP)
                {
                    continue;
                }
                auto payload = std::get<std::pair<uint32_t, PC::pc_map_t>>(packet.getBody().getPayload());
                backup_map = payload.second;
                Threads::Signals::table_version = payload.first;
            }
        }
        conn.close();
    }
} // namespace Subservices::Replication
