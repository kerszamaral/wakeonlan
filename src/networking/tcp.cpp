#include "networking/tcp.hpp"

#include "common/format.hpp"

TCP::TCP(IPv4 ip, Port port) : Socket(Socket::Type::TCP)
{
    sockaddr_in addr;
    addr.sin_family = fmt::to_underlying(Socket::IPVersion::IPv4);
    addr.sin_port = port.get_port();
    Socket::set_saddr(addr, ip.to_network_order());

    connect(addr);
}

TCP::~TCP()
{
    close();
}