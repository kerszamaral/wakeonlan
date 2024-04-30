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

    class Address
    {
    private:
        IPv4 ip;
        Port port;
        addr_t addr;

        void init_addr() noexcept
        {
            addr.sin_family = fmt::to_underlying(IPVersion::IPv4);
            port.setAddrPort(addr);
            ip.to_addr(addr);
        }

    public:
        Address() noexcept : ip(), port() { init_addr(); }
        Address(const std::string &address)
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
        Address(const IPv4 &ip, const Port &port) noexcept : ip(ip), port(port) { init_addr(); }
        Address(const std::string &ip, const Port &port) : ip(ip), port(port) { init_addr(); }
        Address(const IPv4 &ip, const port_t &port) noexcept : ip(ip), port(port) { init_addr(); }
        Address(const std::string &ip, const port_t &port) : ip(ip), port(port) { init_addr(); }
        Address(const addr_t &address) noexcept : ip(address), port(address), addr(address) {}
        Address(const Port &port) noexcept : ip(), port(port) { init_addr(); }
        Address(const port_t &port) noexcept : ip(), port(port) { init_addr(); }
        Address(const IPv4 &ip) noexcept : ip(ip), port() { init_addr(); }

        constexpr void setIP(const IPv4 &new_ip) noexcept
        {
            ip = new_ip;
            ip.to_addr(addr);
        }
        void setIP(const std::string &new_ip) { setIP(IPv4(new_ip)); }
        void setPort(const Port &new_port) noexcept
        {
            port = new_port;
            port.setAddrPort(addr);
        }

        void setPort(const port_t &new_port) noexcept { setPort(Port(new_port)); }
        void setAddr(const addr_t &new_addr) noexcept
        {
            addr = new_addr;
            ip = IPv4(addr);
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
