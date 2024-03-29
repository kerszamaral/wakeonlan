#include "networking/connection.hpp"

Connection::Connection(IPv4 ip, Port port) : Socket()
{
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = port.get_port();
    Socket::set_saddr(addr, ip.to_network_order());

    connect(addr);
}

Connection::~Connection()
{
    close();
}