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

        const auto our_ip = Networking::Addresses::IPv4::FromMachine();
    
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
                const auto different_manager = Listen::listen_for_clients(conn, ssr, pc_map, sleep_status, our_ip);
                if (different_manager)
                {
                    Threads::Signals::is_manager = false;
                    Threads::Signals::update = true;
                    Threads::Signals::update.notify_all();
                }
            }
            else
            {
                // Delay will already be handled in listen_for_managers
                const auto &maybe_packet = conn.wait_and_receive_packet(Threads::Delays::CHECK_DELAY);
                if (!maybe_packet.has_value())
                {
                    if (Threads::Signals::current_manager != 0 && (std::chrono::steady_clock::now() - manager_last_seen > Threads::Delays::MANAGER_TIMEOUT))
                    {
                        pc_map.execute(remove_manager);
                        Threads::Signals::current_manager = 0;
                        Threads::Signals::current_manager.notify_all();
                        Threads::Signals::update = true;
                        Threads::Signals::update.notify_all();
                    }
                }
                else
                {
                    auto &[packet, addr] = maybe_packet.value();
                    if (packet.getType() != Packets::PacketType::SSR)
                    {
                        continue;
                    }
                    const uint32_t current_manager = Threads::Signals::current_manager;
                    const auto ssr_ack = Packets::Packet(Packets::PacketType::SSR_ACK, current_manager);
                    conn.send(ssr_ack, addr);
                    manager_last_seen = std::chrono::steady_clock::now();
                }
            }
            std::this_thread::sleep_for(Threads::Delays::CHECK_DELAY);
        }
        conn.close();
    }
}