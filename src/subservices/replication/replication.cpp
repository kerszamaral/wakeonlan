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
            if (pc.get_hostname() == our_hostname)
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
        Threads::Signals::update = true;
        Threads::Signals::update.notify_all();
    }

    void initialize(PC::atomic_pc_map_t &pc_map, PC::updates_queue &updates)
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

        bool send_update = false;
        while (Threads::Signals::run)
        {
            const auto is_manager = Threads::Signals::is_manager.load();
            if (was_manager != is_manager)
            {
                was_manager = is_manager;
                if (was_manager)
                {
                    // We are now the manager
                    pc_map.execute(copy_backup_to_main, backup_map, our_hostname);
                    send_update = true;
                }
                else
                {
                    // We are no longer the manager
                    pc_map.execute(remove_everything_but_manager);
                    send_update = false;
                }
                // while (conn.wait_and_receive_packet(Threads::Delays::FLUSH_DELAY).has_value())
                //     ; // Clear the queue
            }

            if (is_manager)
            {
                // Manager sends the table to all clients
                auto maybe_update = updates.consume();
                if (maybe_update.has_value())
                {
                    auto [update_type, pc_info] = maybe_update.value();
                    switch (update_type)
                    {
                    case PC::UPDATE_TYPE::ADD:
                        backup_map.insert_or_assign(pc_info.get_hostname(), pc_info);
                        break;
                    case PC::UPDATE_TYPE::REMOVE:
                        backup_map.erase(pc_info.get_hostname());
                        pc_map.execute([&pc_info](PC::pc_map_t &pc_map){ pc_map.erase(pc_info.get_hostname());});
                        Threads::Signals::update = true;
                        Threads::Signals::update.notify_all();
                        break;
                    case PC::UPDATE_TYPE::CHANGE:
                        backup_map.insert_or_assign(pc_info.get_hostname(), pc_info);
                        break;
                    default:
                        break;
                    }
                    send_update = true;
                }
                
                if (send_update)
                {
                    Threads::Signals::table_version++;
                    const uint32_t table_version = Threads::Signals::table_version;
                    auto payload = std::make_pair(table_version, backup_map);
                    auto packet = Packets::Packet(Packets::PacketType::SSREP, payload);
                    conn.send_broadcast(packet, Addresses::REPLICATION_PORT);
                    // std::cout << "tv: " << table_version << std::endl;
                    send_update = false;
                }
                else
                {
                    std::this_thread::sleep_for(Threads::Delays::CHECK_DELAY);
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
