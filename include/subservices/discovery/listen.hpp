#pragma once

#include "common/pcinfo.hpp"
#include "networking/sockets/udp.hpp"
#include "networking/packet.hpp"

namespace Subservices::Discovery::Listen
{
    void listen_for_clients(const Networking::Packet &discovery_packet, Networking::Sockets::UDP &conn, PC::new_pcs_queue &new_pcs);
}