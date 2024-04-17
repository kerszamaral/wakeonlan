#include "subservices/discovery/listen.hpp"

namespace Subservices::Discovery::Listen
{
    void listen_for_clients(const Networking::Packet &discovery_packet, Networking::Sockets::UDP &conn, PC::new_pcs_queue &new_pcs)
    {
        constexpr const auto WAIT_DELAY = std::chrono::milliseconds(100);
        auto pack = conn.wait_and_receive_packet(WAIT_DELAY);
        if (!pack.has_value())
        {
            return;
        }
        auto [packet, addr] = pack.value();
        if (packet.getType() != Networking::PacketType::SSD)
        {
            return; // Received packet was not an SSD packet
        }
        // From here, we can assume that the packet is a discovery packet
        auto [packet_hostname, packet_mac] = std::get<Networking::SSE_Data>(packet.getBody().getPayload());

        // Add the PC to the queue
        // (print for now)
        PC::PCInfo client(packet_hostname, packet_mac, addr.getIp(), PC::STATUS::AWAKE);
        new_pcs.produce(client);

        // Send a response packet
        conn.send(discovery_packet, addr);
    }
}