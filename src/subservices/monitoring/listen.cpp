#include "subservices/monitoring/listen.hpp"

#include "threads/signals.hpp"

namespace Subservices::Monitoring::Listen
{

    PC::STATUS wait_for_response(Networking::Sockets::UDP &conn)
    {
        constexpr const auto WAIT_DELAY = std::chrono::milliseconds(1000);
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

    void listen_for_clients(Networking::Sockets::UDP &conn, const Networking::Packets::Packet &ssr, PC::atomic_pc_map_t &pc_map, PC::sleep_queue &sleep_status)
    {
        //? We copy the usedful information from the pcs to not lock the pc_map for too long
        std::vector<std::pair<PC::hostname_t, Networking::Addresses::IPv4>> local_pc_map;
        const auto get_pcs = [&local_pc_map](const PC::pc_map_t &pc_map) -> void
        {
            for (const auto &[hostname, pc_info] : pc_map)
            {
                local_pc_map.emplace_back(hostname, pc_info.get_ipv4());
            }
        };
        pc_map.execute(get_pcs);
        if (local_pc_map.empty())
        {
            return; // We have no pcs to send to
        }

        Networking::Addresses::Address addr; // So we avoid creating a new Address object on each iteration
        addr.setPort(Networking::Addresses::MONITOR_PORT);
        // std::cout << "sending on " << addr << std::endl;
        for (auto &[hostname, ipv4] : local_pc_map)
        {
            // Early exit if we are told to stop
            if (!Threads::Signals::run)
            {
                break;
            }
            addr.setIP(ipv4);
            std::cout << "Sending to " << addr << std::endl;
            std::cout << "testing " << hostname << std::endl;
            conn.send(ssr, addr);
            const auto &pc_status = wait_for_response(conn);
            //? Maybe we should have a second chance algorithm here
            sleep_status.produce({hostname, pc_status});
            std::cout << hostname << " is " << pc_status << std::endl;
        }
    }
}