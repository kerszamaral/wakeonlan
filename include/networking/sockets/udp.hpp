#pragma once

#include <optional>
#include <tuple>

#include "networking/packet.hpp"
#include "networking/sockets/socket.hpp"

namespace Networking::Sockets
{
    class UDP : public Socket
    {
    protected:
        void send(const payload_t &message, const Networking::Addresses::Address &addr) const;
        std::optional<std::pair<payload_t, Networking::Addresses::Address>> receive() const;
        std::optional<std::pair<payload_t, Networking::Addresses::Address>> wait_and_receive(uint32_t timeout) const;
        std::pair<payload_t, Networking::Addresses::Address> wait_and_receive() const;

    public:
        UDP() : Socket(Type::UDP) { this->setNonBlocking(true); }
        // UDP(socket_t s) : Socket(s) {}
        UDP(const Networking::Addresses::Port &port);
        UDP(uint16_t port) : UDP(Networking::Addresses::Port(port)) {}
        ~UDP();

        void send(const Networking::Packet &packet, const Networking::Addresses::Address &addr) const;
        std::optional<std::pair<Networking::Packet, Networking::Addresses::Address>> receive_packet() const;
        std::optional<std::pair<Networking::Packet, Networking::Addresses::Address>> wait_and_receive_packet(uint32_t timeout) const;
        std::pair<Networking::Packet, Networking::Addresses::Address> wait_and_receive_packet() const;

        void send_broadcast(const Networking::Packet &packet, const Networking::Addresses::Port &port);
        void send_broadcast(const Networking::Packet &packet, uint16_t port) { send_broadcast(packet, Networking::Addresses::Port(port)); }

        static void broadcast(const Networking::Packet &packet, const Networking::Addresses::Port &port);
        static void broadcast(const Networking::Packet &packet, uint16_t port) { broadcast(packet, Networking::Addresses::Port(port)); }
    };
}