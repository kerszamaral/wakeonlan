#include "subservices/monitoring/listen.hpp"

#include <thread>

#include "threads/signals.hpp"

namespace Subservices::Monitoring::Listen
{

    PC::STATUS wait_for_response(Networking::Sockets::UDP &conn)
    {
        constexpr const auto WAIT_DELAY = std::chrono::milliseconds(100);
        while (Threads::Signals::run) // We will break out of the loop when we receive a response
        {
            const auto &maybe_packet = conn.wait_and_receive_packet(WAIT_DELAY);
            if (!maybe_packet.has_value())
            {
                return PC::STATUS::SLEEPING; // No response received
            }
            auto &[packet, addr] = maybe_packet.value();
            if (packet.getType() != Networking::Packets::PacketType::SSR_ACK)
            {
                continue; // For some reason, the packet was not an SSD_ACK packet
            }
            return PC::STATUS::AWAKE; // We have received a response
        }
        return PC::STATUS::UNKNOWN; // We have been told to stop listening
    }

    auto get_pcs(const PC::pc_map_t &pc_map)
    {
        std::vector<std::tuple<PC::hostname_t, Networking::Addresses::IPv4, PC::STATUS>> pcs;
        for (const auto &[hostname, pc_info] : pc_map)
        {
            pcs.emplace_back(hostname, pc_info.get_ipv4(), pc_info.get_status());
        }
        return pcs;
    }

    void listen_for_clients(Networking::Sockets::UDP &conn, const Networking::Packets::Packet &ssr, PC::atomic_pc_map_t &pc_map, PC::sleep_queue &sleep_status)
    {
        //? We copy the usedful information from the pcs to not lock the pc_map for too long
        const auto &local_pc_map = pc_map.execute(get_pcs);
        if (local_pc_map.empty())
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            return; // We have no pcs to send to
        }

        Networking::Addresses::Address addr(Networking::Addresses::MONITOR_PORT); // So we avoid creating a new Address object on each iteration
        for (auto &[hostname, ipv4, status] : local_pc_map)
        {
            // Early exit if we are told to stop
            if (!Threads::Signals::run)
            {
                break;
            }
            addr.setIP(ipv4);
            conn.send(ssr, addr);
            const auto &pc_status = wait_for_response(conn);
            // Only add to the sleep_status queue if the status has changed
            if (pc_status != status)
            {
                //? Maybe we should have a second chance algorithm here
                sleep_status.produce({hostname, pc_status});
            }
        }
    }
}