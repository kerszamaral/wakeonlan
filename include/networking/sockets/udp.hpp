#pragma once

#include "networking/sockets/socket.hpp"

namespace Networking::Sockets
{
    class UDP : public Socket
    {
    private:
        Networking::Addresses::Address addr;

    public:
        UDP(const Networking::Addresses::Address &address);
        UDP(const std::string &address) : UDP(Networking::Addresses::Address(address)) {}
        ~UDP();
    };
}