#pragma once

#include <optional>
#include <tuple>
#include <functional>

#include "networking/packet.hpp"
#include "networking/sockets/socket.hpp"

namespace Networking::Sockets
{
    class UDP : protected Socket
    {
    protected:
        std::optional<std::reference_wrapper<UDP>> send(const payload_t &message, const Networking::Addresses::Address &addr);
        std::optional<std::pair<payload_t, Networking::Addresses::Address>> receive();
        std::optional<std::pair<payload_t, Networking::Addresses::Address>> wait_and_receive(uint32_t timeout);
        std::optional<std::pair<payload_t, Networking::Addresses::Address>> wait_and_receive();

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

        std::optional<std::reference_wrapper<UDP>> send(const Networking::Packet &packet, const Networking::Addresses::Address &addr);
        std::optional<std::pair<Networking::Packet, Networking::Addresses::Address>> receive_packet();
        std::optional<std::pair<Networking::Packet, Networking::Addresses::Address>> wait_and_receive_packet(uint32_t timeout);
        std::optional<std::pair<Networking::Packet, Networking::Addresses::Address>> wait_and_receive_packet();

        std::optional<std::reference_wrapper<UDP>> send_broadcast(const Networking::Packet &packet, const Networking::Addresses::Port &port);
        std::optional<std::reference_wrapper<UDP>> send_broadcast(const Networking::Packet &packet, uint16_t port) { return send_broadcast(packet, Networking::Addresses::Port(port)); }

        success_t close() { return Socket::close(); }

        static success_t broadcast(const Networking::Packet &packet, const Networking::Addresses::Port &port);
        static success_t broadcast(const Networking::Packet &packet, uint16_t port) { return broadcast(packet, Networking::Addresses::Port(port)); }
    };
}