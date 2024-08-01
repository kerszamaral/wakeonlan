#include "subservices/discovery/listen.hpp"

#include "threads/delays.hpp"

namespace Subservices::Discovery::Listen
{
    void listen_for_clients(const Networking::Packets::Packet &discovery_packet, Networking::Sockets::UDP &conn, PC::new_pcs_queue &new_pcs, const PC::hostname_t &our_hostname)
    {
        auto pack = conn.wait_and_receive_packet(Threads::Delays::WAIT_DELAY);
        if (!pack.has_value())
        {
            return;
        }
        auto [packet, addr] = pack.value();
        if (packet.getType() != Networking::Packets::PacketType::SSD)
        {
            return; // Received packet was not an SSD packet
        }
        // From here, we can assume that the packet is a discovery packet
        auto [packet_hostname, packet_mac] = std::get<Networking::Packets::SSE_Data>(packet.getBody().getPayload());

        if (packet_hostname == our_hostname)
        {
            return; // We received our own packet
        }

        // Add the PC to the queue
        // (print for now)
        PC::PCInfo client(packet_hostname, packet_mac, addr.getIp(), PC::STATUS::AWAKE);
        new_pcs.produce(client);

        // Send a response packet
        conn.send(discovery_packet, addr);
    }
}