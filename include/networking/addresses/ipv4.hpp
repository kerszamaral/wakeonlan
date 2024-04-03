#pragma once

#include <string>
#include <cstdint>
#include <array>

namespace Networking::Addresses
{
    typedef uint8_t ipv4_t;
    constexpr auto BROADCAST_IP = "255.255.255.255";

    class IPv4
    {
    private:
        constexpr static char IPV4_ADDR_DELIM = '.';
        constexpr static int IPV4_ADDR_LEN = 4;
        std::array<ipv4_t, IPV4_ADDR_LEN> m_ipv4_addr;

    public:
        IPv4() : IPv4(0) {}
        IPv4(const std::string &ipv4_addr);
        IPv4(const uint32_t &ipv4_addr) : m_ipv4_addr(from_network_order(ipv4_addr)) {}
        IPv4(const std::array<ipv4_t, IPV4_ADDR_LEN> &ipv4_addr) : IPv4(array_to_network_order(ipv4_addr)) {}
        ~IPv4();

        uint32_t to_network_order() const;
        std::string to_string() const;

        friend std::ostream &operator<<(std::ostream &os, const IPv4 &ipv4);
        static std::array<ipv4_t, IPV4_ADDR_LEN> from_network_order(const uint32_t &ipv4_addr);
        static uint32_t array_to_network_order(const std::array<ipv4_t, IPV4_ADDR_LEN> &ipv4_addr);
    };
}