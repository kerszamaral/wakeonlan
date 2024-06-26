#include "subservices/discovery/discovery.hpp"

#include <thread>
#include <variant>

#include "common/pcinfo.hpp"
#include "threads/delays.hpp"
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
    using namespace Networking;
    using namespace Packets;

    void initialize(PC::new_pcs_queue &new_pcs)
    {
        //? Building the used packets
        Packet disc_packet(PacketType::SSD);
        Packet disc_ack_packet(PacketType::SSD_ACK);

        //? Opening UDP connection
        auto conn = Sockets::UDP(Addresses::DISCOVERY_PORT);

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
                Listen::listen_for_clients(disc_ack_packet, conn, new_pcs);
            }
            else
            {
                // Try to discover the manager
                Threads::Signals::manager_found = Find::find_manager(conn, new_pcs);
                if (!Threads::Signals::manager_found)
                {
                    conn.send_broadcast(disc_packet, Addresses::DISCOVERY_PORT);
                }
                Threads::Signals::manager_found.wait(true);
            }
            std::this_thread::sleep_for(Threads::Delays::WAIT_DELAY);
        }
        conn.close();
    }
}