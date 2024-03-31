#pragma once

#include "networking/socket.hpp"
#include "networking/port.hpp"
#include "networking/tcp.hpp"

namespace Socket
{
    class PortListener
    {
    private:
        Socket server = Socket(Type::TCP);
        Address addr;
        int opt = 1;

    public:
        PortListener(Port port);
        PortListener(uint16_t port) : PortListener(Port(port)) {}
        TCP waitForConnection();
        ~PortListener();
    };
}