#include "subservices/discovery/find.hpp"

#include "networking/packets/packet.hpp"
#include "threads/signals.hpp"

namespace Subservices::Discovery::Find
{
    bool find_manager(Networking::Sockets::UDP &conn, PC::new_pcs_queue &new_pcs)
    {
        constexpr const auto WAIT_DELAY = std::chrono::milliseconds(100);
        do
        {
            auto resp = conn.wait_and_receive_packet(WAIT_DELAY);
            if (!resp.has_value())
            {
                return false; // No response received
            }
            auto [packet, addr] = resp.value();
            // While we have packets in the queue, we check if they are SSD_ACK packets
            // if not, we continue to the next packet
            if (packet.getType() != Networking::Packets::PacketType::SSD_ACK)
            {
                continue; // Received packet was not an SSD_ACK packet
            }
            // From here, we can assume that the packet is a discovery packet
            auto [packet_hostname, packet_mac] = std::get<Networking::Packets::SSE_Data>(packet.getBody().getPayload());

            // Add the PC to the queue
            PC::PCInfo manager(packet_hostname, packet_mac, addr.getIp(), PC::STATUS::AWAKE, true);
            new_pcs.produce(manager);
            // manager has been found
            return true;
        } while (Threads::Signals::run);
        return false;
    }
}