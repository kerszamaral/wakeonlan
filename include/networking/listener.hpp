#pragma once

#include "networking/socket.hpp"
#include "networking/port.hpp"
#include "networking/connection.hpp"

class PortListener
{
private:
    Socket server;
    sockaddr_in addr;
    int opt = 1;

public:
    PortListener(Port port);
    PortListener(uint16_t port) : PortListener(Port(port)) {}
    Connection waitForConnection();
    ~PortListener();
};