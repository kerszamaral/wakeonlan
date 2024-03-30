#pragma once

#include "networking/ipv4.hpp"
#include "networking/port.hpp"
#include "networking/socket.hpp"

class UDP : public Socket
{
private:
    sockaddr_in addr;

public:
    UDP(socket_t s) : Socket(s) {}
    UDP(IPv4 ip, Port port);
    UDP(std::string ip, Port port) : UDP(IPv4(ip), port) {}
    UDP(std::string ip, uint16_t port) : UDP(IPv4(ip), Port(port)) {}
    UDP(IPv4 ip, uint16_t port) : UDP(ip, Port(port)) {}
    ~UDP();

    void send(std::string message) const;
};