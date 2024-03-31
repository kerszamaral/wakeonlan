#pragma once

#include <string>
#include <cstdint>
#include <array>

namespace Networking::Addresses
{
    typedef uint8_t ipv4_t;

    class IPv4
    {
    private:
        constexpr static char IPV4_ADDR_DELIM = '.';
        constexpr static int IPV4_ADDR_LEN = 4;
        std::array<ipv4_t, IPV4_ADDR_LEN> m_ipv4_addr;

    public:
        IPv4() : IPv4(0) {}
        IPv4(const std::string &ipv4_addr);
        IPv4(const uint32_t &ipv4_addr);
        ~IPv4();

        uint32_t to_network_order() const;
        std::string to_string() const;

        friend std::ostream &operator<<(std::ostream &os, const IPv4 &ipv4);
    };
}