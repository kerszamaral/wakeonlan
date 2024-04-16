#include "discovery/discovery.hpp"

#include <thread>
#include <variant>

#include "common/pcinfo.hpp"
#include "threads/signals.hpp"
#include "networking/sockets/udp.hpp"

/*
This subservice discovers new PCs on the network
and adds then them to the queue to be added to the pc_map
*/
namespace Addr = Networking::Addresses;
namespace Sockets = Networking::Sockets;

using Packet = Networking::Packet;
using UDPConn = Sockets::UDP;
using Port = Addr::Port;
using MacAddr = Networking::MacAddress;
using PacketType = Networking::PacketType;

bool find_manager(UDPConn &conn, Threads::ProdCosum<PCInfo> &new_pcs);

void listen_for_clients(const Packet &discovery_packet, UDPConn &conn, const Port &discovery_port, Threads::ProdCosum<PCInfo> &new_pcs);

void init_discovery(Threads::ProdCosum<PCInfo> &new_pcs)
{
    constexpr const auto CHECK_DELAY = std::chrono::milliseconds(100);
    //? Port and Address setup
    constexpr uint16_t disc_port_num = Port::DISCOVERY_PORT;
    Port discovery_port(disc_port_num);

    //? Our UDP connection side and packets
    UDPConn conn = UDPConn(disc_port_num);
    const auto hostname = PCInfo::getMachineName();
    const auto mac = MacAddr::getMacAddr().value();
    const auto data = std::make_pair(hostname, mac);
    Packet discovery_packet(PacketType::SSD, data);
    Packet discovery_ack_packet(PacketType::SSD_ACK, data);

    while (Threads::Signals::run.load())
    {
        if (Threads::Signals::is_manager.load())
        {
            // Discover new PCs
            // Add them to the queue
            // Set update to true
            // Listen for clients
            listen_for_clients(discovery_ack_packet, conn, discovery_port, new_pcs);
        }
        else
        {
            // Try to discover the manager
            if (!Threads::Signals::manager_found.load())
            {
                const bool &found = find_manager(conn, new_pcs);
                Threads::Signals::manager_found.store(found);
                if (!Threads::Signals::manager_found.load())
                {
                    conn.send_broadcast(discovery_packet, discovery_port);
                }
            }
        }
        std::this_thread::sleep_for(CHECK_DELAY);
    }
    conn.close();
}

bool find_manager(UDPConn &conn, Threads::ProdCosum<PCInfo> &new_pcs)
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
        if (packet.getType() != PacketType::SSD_ACK)
        {
            continue; // Received packet was not an SSD_ACK packet
        }
        // From here, we can assume that the packet is a discovery packet
        auto [packet_hostname, packet_mac] = std::get<Networking::SSE_Data>(packet.getBody().getPayload());

        // Add the PC to the queue
        PCInfo manager(packet_hostname, packet_mac, addr.getIp(), PC_STATUS::AWAKE, true);
        new_pcs.produce(manager);
        // manager has been found
        return true;
    } while (Threads::Signals::run.load());
    return false;
}

void listen_for_clients(const Packet &discovery_packet, UDPConn &conn, const Port &discovery_port, Threads::ProdCosum<PCInfo> &new_pcs)
{
    constexpr const auto WAIT_DELAY = std::chrono::milliseconds(100);
    auto pack = conn.wait_and_receive_packet(WAIT_DELAY);
    if (!pack.has_value())
    {
        return;
    }
    auto [packet, addr] = pack.value();
    if (packet.getType() != PacketType::SSD)
    {
        return; // Received packet was not an SSD packet
    }
    // From here, we can assume that the packet is a discovery packet
    auto [packet_hostname, packet_mac] = std::get<Networking::SSE_Data>(packet.getBody().getPayload());

    // Add the PC to the queue
    // (print for now)
    PCInfo client(packet_hostname, packet_mac, addr.getIp(), PC_STATUS::AWAKE);
    new_pcs.produce(client);

    // Send a response packet
    conn.send(discovery_packet, addr);
}