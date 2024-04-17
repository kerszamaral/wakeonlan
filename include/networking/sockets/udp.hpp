#pragma once

#include <optional>
#include <tuple>
#include <functional>
#include <chrono>

#include "networking/packets/packet.hpp"
#include "networking/sockets/socket.hpp"

namespace Networking::Sockets
{
    class UDP : protected Socket
    {
    protected:
        opt::optional<std::reference_wrapper<UDP>> send(const Packets::payload_t &message, const Networking::Addresses::Address &addr);
        opt::optional<std::pair<Packets::payload_t, Networking::Addresses::Address>> receive();
        opt::optional<std::pair<Packets::payload_t, Networking::Addresses::Address>> wait_and_receive(std::chrono::milliseconds timeout);
        opt::optional<std::pair<Packets::payload_t, Networking::Addresses::Address>> wait_and_receive();

    public:
        UDP() : Socket(Type::UDP)
        {
            if (!this->setNonBlocking(true).has_value())
                throw socket_error("Error setting nonblocking");
        }
        // UDP(socket_t s) : Socket(s) {}
        UDP(const Networking::Addresses::Port &port);
        UDP(uint16_t port) : UDP(Networking::Addresses::Port(port)) {}
        ~UDP();

        opt::optional<std::reference_wrapper<UDP>> send(const Networking::Packets::Packet &packet, const Networking::Addresses::Address &addr);
        opt::optional<std::pair<Networking::Packets::Packet, Networking::Addresses::Address>> receive_packet();
        opt::optional<std::pair<Networking::Packets::Packet, Networking::Addresses::Address>> wait_and_receive_packet(std::chrono::milliseconds timeout);
        opt::optional<std::pair<Networking::Packets::Packet, Networking::Addresses::Address>> wait_and_receive_packet();

        opt::optional<std::reference_wrapper<UDP>> send_broadcast(const Networking::Packets::Packet &packet, const Networking::Addresses::Port &port);
        opt::optional<std::reference_wrapper<UDP>> send_broadcast(const Networking::Packets::Packet &packet, uint16_t port) { return send_broadcast(packet, Networking::Addresses::Port(port)); }
        opt::optional<std::reference_wrapper<UDP>> send_wakeup(const Networking::Addresses::Mac &mac);

        success_t close() { return Socket::close(); }

        static success_t broadcast(const Networking::Packets::Packet &packet, const Networking::Addresses::Port &port);
        static success_t broadcast(const Networking::Packets::Packet &packet, uint16_t port) { return broadcast(packet, Networking::Addresses::Port(port)); }
        static success_t broadcast_wakeup(const Networking::Addresses::Mac &mac);
    };
}