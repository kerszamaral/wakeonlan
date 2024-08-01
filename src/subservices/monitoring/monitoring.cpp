#include "subservices/monitoring/monitoring.hpp"

#include <thread>

#include "threads/delays.hpp"
#include "threads/signals.hpp"
#include "networking/sockets/udp.hpp"
#include "subservices/monitoring/listen.hpp"

namespace Subservices::Monitoring
{
    void remove_manager(PC::pc_map_t &pc_map)
    {
        for (auto &[hostname, pc] : pc_map)
        {
            if (pc.get_is_manager())
            {
                pc_map.erase(hostname);
                break;
            }
        }
    }

    void initialize(PC::atomic_pc_map_t &pc_map, PC::sleep_queue &sleep_status)
    {
        using namespace Networking;
        auto conn = Sockets::UDP(Addresses::MONITOR_PORT);
        auto ssr = Packets::Packet(Packets::PacketType::SSR);
        auto ssr_ack = Packets::Packet(Packets::PacketType::SSR_ACK);

        bool transition = Threads::Signals::is_manager;

        auto manager_last_seen = std::chrono::steady_clock::now();

        while (Threads::Signals::run)
        {
            if (transition != Threads::Signals::is_manager)
            {
                transition = Threads::Signals::is_manager;
                if (Threads::Signals::is_manager)
                {
                    while (conn.wait_and_receive_packet(Threads::Delays::FLUSH_DELAY).has_value())
                        ; // Clear the queue
                }
            }

            if (Threads::Signals::is_manager)
            {
                Listen::listen_for_clients(conn, ssr, pc_map, sleep_status);
            }
            else
            {
                // Delay will already be handled in listen_for_managers
                const auto &maybe_packet = conn.wait_and_receive_packet(Threads::Delays::CHECK_DELAY);
                if (!maybe_packet.has_value())
                {
                    if (Threads::Signals::manager_found && (std::chrono::steady_clock::now() - manager_last_seen > Threads::Delays::MANAGER_TIMEOUT))
                    {
                        pc_map.execute(remove_manager);
                        Threads::Signals::manager_found = false;
                        Threads::Signals::manager_found.notify_all();
                        Threads::Signals::update = true;
                        Threads::Signals::update.notify_all();
                        Threads::Signals::replication_update = true;
                        Threads::Signals::replication_update.notify_all();
                    }
                }
                else
                {
                    auto &[packet, addr] = maybe_packet.value();
                    if (packet.getType() != Packets::PacketType::SSR)
                    {
                        continue;
                    }
                    manager_last_seen = std::chrono::steady_clock::now();
                    conn.send(ssr_ack, addr);
                }
            }
            std::this_thread::sleep_for(Threads::Delays::CHECK_DELAY);
        }
        conn.close();
    }
}