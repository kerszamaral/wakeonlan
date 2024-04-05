#include "networking/addresses/address.hpp"

#include "common/format.hpp"

namespace Networking::Addresses
{
    void set_saddr(addr_t &addr, const uint32_t &new_addr)
    {
#ifdef _WIN32
        addr.sin_addr.S_un.S_addr = new_addr;
#else
        addr.sin_addr.s_addr = new_addr;
#endif
    }

    uint32_t get_saddr(const addr_t &addr)
    {
#ifdef _WIN32
        return addr.sin_addr.S_un.S_addr;
#else
        return addr.sin_addr.s_addr;
#endif
    }

    void Address::init_addr()
    {
        addr.sin_family = fmt::to_underlying(IPVersion::IPv4);
        addr.sin_port = port.getPort();
        set_saddr(addr, ip.to_network_order());
    }

    Address::Address(const std::string &address)
    {
        auto colon = address.find(':');
        if (colon == std::string::npos)
        {
            throw std::runtime_error("Invalid address");
        }
        ip = IPv4(address.substr(0, colon));
        port = Port(address.substr(colon + 1));
        init_addr();
    }

    void Address::setIP(const IPv4 &new_ip)
    {
        ip = new_ip;
        set_saddr(addr, ip.to_network_order());
    }

    void Address::setPort(const Port &new_port)
    {
        port = new_port;
        addr.sin_port = port.getPort();
    }

    void Address::setAddr(const addr_t &new_addr)
    {
        addr = new_addr;
        ip = IPv4(get_saddr(addr));
        port = Port(addr.sin_port);
    }

    std::string Address::to_string() const
    {
        return ip.to_string() + ":" + port.to_string();
    }

    std::ostream &operator<<(std::ostream &os, const Address &a)
    {
        os << a.to_string();
        return os;
    }
}