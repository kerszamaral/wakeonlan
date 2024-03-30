#include "networking/udp.hpp"

#include "common/format.hpp"

UDP::UDP(IPv4 ip, Port port) : Socket(Socket::Type::UDP)
{
    addr.sin_family = fmt::to_underlying(Socket::IPVersion::IPv4);
    addr.sin_port = port.get_port();
    Socket::set_saddr(addr, ip.to_network_order());
}

void UDP::send(std::string message) const
{
    Socket::sendto(message, addr);
}

UDP::~UDP()
{
    close();
}