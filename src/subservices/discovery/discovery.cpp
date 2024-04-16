#include "subservices/discovery/discovery.hpp"

#include <thread>
#include <variant>

#include "common/pcinfo.hpp"
#include "threads/signals.hpp"
#include "networking/sockets/udp.hpp"

#include "subservices/discovery/listen.hpp"
#include "subservices/discovery/find.hpp"

/*
This subservice discovers new PCs on the network
and adds then them to the queue to be added to the pc_map
*/
namespace Subservices::Discovery
{
    using Packet = Networking::Packet;
    using PacketType = Networking::PacketType;
    using UDPSocket = Networking::Sockets::UDP;
    using Port = Networking::Addresses::Port;

    void initialize(PC::new_pcs_queue &new_pcs)
    {
        //? Port and Address setup
        constexpr uint16_t disc_port_num = Port::DISCOVERY_PORT;
        Port discovery_port(disc_port_num);

        //? Our UDP connection side and packets
        UDPSocket conn = UDPSocket(disc_port_num);
        const auto hostname = PC::getHostname();
        const auto mac = Networking::Addresses::Mac::FromMachine().value();
        const auto data = std::make_pair(hostname, mac);
        Packet discovery_packet(PacketType::SSD, data);
        Packet discovery_ack_packet(PacketType::SSD_ACK, data);

        while (Threads::Signals::run)
        {
            if (Threads::Signals::is_manager)
            {
                Listen::listen_for_clients(discovery_ack_packet, conn, discovery_port, new_pcs);
            }
            else
            {
                // Try to discover the manager
                Threads::Signals::manager_found = Find::find_manager(conn, new_pcs);
                if (!Threads::Signals::manager_found)
                {
                    conn.send_broadcast(discovery_packet, discovery_port);
                }
                Threads::Signals::manager_found.wait(true);
            }
        }
        conn.close();
    }
}