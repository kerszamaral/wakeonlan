#pragma once

#include "networking/sockets/socket.hpp"
#include "networking/sockets/tcp.hpp"

namespace Networking::Sockets
{
    class PortListener
    {
    private:
        Socket server = Socket(Type::TCP);
        Networking::Addresses::Address addr;
        int opt = 1;

    public:
        PortListener(Networking::Addresses::Port port);
        PortListener(uint16_t port) : PortListener(Networking::Addresses::Port(port)) {}
        TCP waitForConnection();
        ~PortListener();
    };
}