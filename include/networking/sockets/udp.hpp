#pragma once

#include <optional>

#include "networking/sockets/socket.hpp"

namespace Networking::Sockets
{
    class UDP : public Socket
    {
    public:
        UDP() : Socket(Type::UDP) { this->setNonBlocking(true); }
        UDP(socket_t s) : Socket(s) {}
        ~UDP();

        void send(const std::string &message, const Networking::Addresses::Address &addr) const;
        std::optional<std::string> receive(Networking::Addresses::Address &addr) const;
    };
}