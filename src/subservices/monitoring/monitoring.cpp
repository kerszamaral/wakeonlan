#include "subservices/monitoring/monitoring.hpp"

#include <thread>

#include "threads/delays.hpp"
#include "threads/signals.hpp"
#include "networking/sockets/udp.hpp"
#include "subservices/monitoring/listen.hpp"

namespace Subservices::Monitoring
{
    void initialize(PC::atomic_pc_map_t &pc_map, PC::sleep_queue &sleep_status)
    {
        using namespace Networking;
        auto conn = Sockets::UDP(Addresses::MONITOR_PORT);
        auto ssr = Packets::Packet(Packets::PacketType::SSR);
        auto ssr_ack = Packets::Packet(Packets::PacketType::SSR_ACK);

        bool transition = Threads::Signals::is_manager;

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
                std::this_thread::sleep_for(Threads::Delays::CHECK_DELAY);
                Listen::listen_for_clients(conn, ssr, pc_map, sleep_status);
            }
            else
            {
                // Delay will already be handled in listen_for_managers
                const auto &maybe_packet = conn.wait_and_receive_packet(Threads::Delays::CHECK_DELAY);
                if (!maybe_packet.has_value())
                {
                    continue;
                }
                auto &[packet, addr] = maybe_packet.value();
                if (packet.getType() != Packets::PacketType::SSR)
                {
                    continue;
                }
                conn.send(ssr_ack, addr);
            }
        }
        conn.close();
    }
}