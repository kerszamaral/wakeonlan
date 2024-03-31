#pragma once

#include <optional>
#include <tuple>

#include "networking/sockets/socket.hpp"

namespace Networking::Sockets
{
    class UDP : public Socket
    {
    public:
        UDP() : Socket(Type::UDP) { this->setNonBlocking(true); }
        // UDP(socket_t s) : Socket(s) {}
        UDP(const Networking::Addresses::Port &port);
        UDP(uint16_t port) : UDP(Networking::Addresses::Port(port)) {}
        ~UDP();

        void send(const std::string &message, const Networking::Addresses::Address &addr) const;
        std::optional<std::pair<std::string, Networking::Addresses::Address>> receive() const;
        std::optional<std::pair<std::string, Networking::Addresses::Address>> wait_and_receive(uint32_t timeout) const;
        std::pair<std::string, Networking::Addresses::Address> wait_and_receive() const;
    };
}