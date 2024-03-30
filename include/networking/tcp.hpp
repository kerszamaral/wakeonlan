#pragma once

#include "networking/ipv4.hpp"
#include "networking/port.hpp"
#include "networking/socket.hpp"

class TCP : public Socket
{
public:
    TCP(socket_t s) : Socket(s) {}
    TCP(IPv4 ip, Port port);
    TCP(std::string ip, Port port) : TCP(IPv4(ip), port) {}
    TCP(std::string ip, uint16_t port) : TCP(IPv4(ip), Port(port)) {}
    TCP(IPv4 ip, uint16_t port) : TCP(ip, Port(port)) {}
    ~TCP();
};