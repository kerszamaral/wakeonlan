#pragma once

#include "common/pcinfo.hpp"
#include "networking/sockets/udp.hpp"
#include "networking/packets/packet.hpp"

namespace Subservices::Monitoring::Listen
{
    bool listen_for_clients(Networking::Sockets::UDP &conn, const Networking::Packets::Packet &ssr, PC::atomic_pc_map_t &pc_map, PC::sleep_queue &sleep_status, const Networking::Addresses::IPv4 &our_ip);
}