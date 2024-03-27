#pragma once

#include "networking/ipv4.hpp"
#include "networking/port.hpp"
#include "networking/socket.hpp"

class Connection : public Socket
{
public:
    Connection(SOCKET s) : Socket(s) {}
    Connection(IPv4 ip, Port port);
    Connection(std::string ip, Port port) : Connection(IPv4(ip), port) {}
    Connection(std::string ip, uint16_t port) : Connection(IPv4(ip), Port(port)) {}
    Connection(IPv4 ip, uint16_t port) : Connection(ip, Port(port)) {}
    ~Connection();
};