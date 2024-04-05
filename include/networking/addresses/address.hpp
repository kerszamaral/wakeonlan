#pragma once

#include "networking/networking.hpp"

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

    void set_saddr(addr_t &addr, const uint32_t &new_addr);
    uint32_t get_saddr(const addr_t &addr);

    class Address
    {
    private:
        IPv4 ip;
        Port port;
        addr_t addr;

        void init_addr();

    public:
        Address() : ip(), port() { init_addr(); }
        Address(const std::string &address);
        Address(const IPv4 &ip, const Port &port) : ip(ip), port(port) { init_addr(); }
        Address(const std::string &ip, const Port &port) : ip(ip), port(port) { init_addr(); }
        Address(const IPv4 &ip, const uint16_t &port) : ip(ip), port(port) { init_addr(); }
        Address(const std::string &ip, const uint16_t &port) : ip(ip), port(port) { init_addr(); }
        Address(const std::string &ip, const std::string &port) : ip(ip), port(port) { init_addr(); }
        Address(const addr_t &address) : ip(get_saddr(address)), port(address.sin_port), addr(address) {}
        Address(const Address &address) : ip(address.ip), port(address.port), addr(address.addr) {}

        ~Address() {}

        void setIP(const IPv4 &new_ip);
        void setIP(const std::string &new_ip) { setIP(IPv4(new_ip)); }
        void setPort(const Port &new_port);
        void setPort(const uint16_t &new_port) { setPort(Port(new_port)); }
        void setAddr(const addr_t &new_addr);

        IPv4 getIp() const { return ip; }
        Port getPort() const { return port; }
        addr_t getAddr() const { return addr; }

        std::string to_string() const;

        friend std::ostream &operator<<(std::ostream &os, const Address &a);
    };
}
