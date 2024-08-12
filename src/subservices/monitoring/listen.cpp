#include "subservices/monitoring/listen.hpp"

#include <thread>

#include "threads/signals.hpp"
#include "threads/delays.hpp"

namespace Subservices::Monitoring::Listen
{

    std::pair<PC::STATUS, uint32_t> wait_for_response(Networking::Sockets::UDP &conn, const Networking::Addresses::IPv4 &addr)
    {
        while (Threads::Signals::run) // We will break out of the loop when we receive a response
        {
            const auto &maybe_packet = conn.wait_and_receive_packet(Threads::Delays::WAIT_DELAY);
            if (!maybe_packet.has_value())
            {
                return std::make_pair(PC::STATUS::SLEEPING, 0); // No response received
            }
            auto &[packet, src] = maybe_packet.value();
            if (packet.getType() == Networking::Packets::PacketType::SSR)
            {
                Threads::Signals::force_election = true;
                continue; // For some reason, the packet was not an SSD_ACK packet
            }
            if (packet.getType() != Networking::Packets::PacketType::SSR_ACK)
            {
                continue; // For some reason, the packet was not an SSD_ACK packet
            }
            if (addr != src.getAddr())
            {
                continue; // The packet was not from the expected source
            }
            const auto their_current_manager = std::get<uint32_t>(packet.getBody().getPayload());
            return std::make_pair(PC::STATUS::AWAKE, their_current_manager); // We have received a response
        }
        return std::make_pair(PC::STATUS::UNKNOWN, 0);
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

    bool listen_for_clients(Networking::Sockets::UDP &conn, const Networking::Packets::Packet &ssr, PC::atomic_pc_map_t &pc_map, PC::sleep_queue &sleep_status, const Networking::Addresses::IPv4 &our_ip)
    {
        //? We copy the usedful information from the pcs to not lock the pc_map for too long
        const auto &local_pc_map = pc_map.execute(get_pcs);
        if (local_pc_map.empty())
        {
            return false; // We have no pcs to send to
        }
        uint32_t response_count = 0;
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
            const auto &[pc_status, their_manager_ip] = wait_for_response(conn, ipv4);
            // Only add to the sleep_status queue if the status has changed
            if (pc_status != status)
            {
                //? Maybe we should have a second chance algorithm here
                sleep_status.produce({hostname, pc_status});
            }
            if (pc_status == PC::STATUS::AWAKE)
            {
                response_count++;
                if (their_manager_ip != 0 && their_manager_ip != our_ip.to_network_order())
                {
                    return true; // We have found a different manager
                }
            }
        }
        return response_count == 0;
    }
}