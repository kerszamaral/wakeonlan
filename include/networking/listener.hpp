#pragma once

#include "networking/socket.hpp"
#include "networking/port.hpp"
#include "networking/tcp.hpp"

class PortListener
{
private:
    Socket server = Socket(Socket::Type::TCP);
    sockaddr_in addr;
    int opt = 1;

public:
    PortListener(Port port);
    PortListener(uint16_t port) : PortListener(Port(port)) {}
    TCP waitForConnection();
    ~PortListener();
};