#pragma once

#include "common/platform.hpp"

#include "common/format.hpp"
#include "networking/addresses/ipv4.hpp"
#include "networking/addresses/port.hpp"

namespace Networking::Addresses
{
    typedef sockaddr_in addr_t;

    enum class IPVersion : int
    {
        IPv4 = AF_INET,
        IPv6 = AF_INET6
    };

    constexpr void set_saddr(addr_t &addr, const uint32_t &new_addr) noexcept
    {
#ifdef OS_WIN
        addr.sin_addr.S_un.S_addr = new_addr;
#else
        addr.sin_addr.s_addr = new_addr;
#endif
    }

    constexpr uint32_t get_saddr(const addr_t &addr) noexcept
    {
#ifdef OS_WIN
        return addr.sin_addr.S_un.S_addr;
#else
        return addr.sin_addr.s_addr;
#endif
    }

    class Address
    {
    private:
        IPv4 ip;
        Port port;
        addr_t addr;

        constexpr void init_addr() noexcept
        {
            addr.sin_family = fmt::to_underlying(IPVersion::IPv4);
            port.setAddrPort(addr);
            set_saddr(addr, ip.to_network_order());
        }

    public:
        constexpr Address() noexcept : ip(), port() { init_addr(); }
        constexpr Address(const std::string &address)
        {
            auto colon = address.find(':');
            if (colon == std::string::npos)
            {
                throw std::runtime_error("Invalid address");
            }
            ip = IPv4(address.substr(0, colon));
            port = Port(fmt::stoi(address.substr(colon + 1)));
            init_addr();
        }
        constexpr Address(const IPv4 &ip, const Port &port) noexcept : ip(ip), port(port) { init_addr(); }
        constexpr Address(const std::string &ip, const Port &port) : ip(ip), port(port) { init_addr(); }
        constexpr Address(const IPv4 &ip, const port_t &port) noexcept : ip(ip), port(port) { init_addr(); }
        constexpr Address(const std::string &ip, const port_t &port) : ip(ip), port(port) { init_addr(); }
        constexpr Address(const addr_t &address) noexcept : ip(get_saddr(address)), port(address), addr(address) {}
        constexpr Address(const Port &port) noexcept : ip(), port(port) { init_addr(); }
        constexpr Address(const port_t &port) noexcept : ip(), port(port) { init_addr(); }
        constexpr Address(const IPv4 &ip) noexcept : ip(ip), port() { init_addr(); }

        constexpr void setIP(const IPv4 &new_ip) noexcept
        {
            ip = new_ip;
            set_saddr(addr, ip.to_network_order());
        }
        constexpr void setIP(const std::string &new_ip) { setIP(IPv4(new_ip)); }
        constexpr void setPort(const Port &new_port) noexcept
        {
            port = new_port;
            port.setAddrPort(addr);
        }

        constexpr void setPort(const port_t &new_port) noexcept { setPort(Port(new_port)); }
        constexpr void setAddr(const addr_t &new_addr) noexcept
        {
            addr = new_addr;
            ip = IPv4(get_saddr(addr));
            port = Port(addr);
        }

        constexpr IPv4 getIp() const noexcept { return ip; }
        constexpr Port getPort() const noexcept { return port; }
        constexpr addr_t getAddr() const noexcept { return addr; }

        std::string to_string() const
        {
            return ip.to_string() + ":" + port.to_string();
        }

        friend std::ostream &operator<<(std::ostream &os, const Address &a)
        {
            os << a.to_string();
            return os;
        }
        constexpr bool operator==(const Address &other) const noexcept
        {
            return ip == other.ip && port == other.port;
        }
    };
}
