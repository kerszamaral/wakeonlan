#pragma once

#include "networking/sockets/socket.hpp"

namespace Networking::Sockets
{
    class TCP : public Socket
    {
    public:
        TCP(socket_t s) : Socket(s) {}
        TCP(const Networking::Addresses::Address &addr);
        TCP(const std::string &address) : TCP(Networking::Addresses::Address(address)) {}
        ~TCP();
    };
}