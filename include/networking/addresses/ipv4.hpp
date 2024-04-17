#pragma once

#include <string>
#include <cstdint>
#include <array>
#include <algorithm>
#include <ranges>
#include "common/format.hpp"

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
        constexpr IPv4() : IPv4(0) {}
        constexpr IPv4(const std::array<ipv4_t, IPV4_ADDR_LEN> &ipv4_addr) : m_ipv4_addr(ipv4_addr) {}
        constexpr IPv4(const uint32_t &ipv4_addr) : m_ipv4_addr(from_network_order(ipv4_addr)) {}
        IPv4(const std::string &ipv4_addr)
        {
            if (ipv4_addr.empty())
            {
                throw std::invalid_argument("IPv4 address cannot be empty");
            }

            auto byte_vector = fmt::split(ipv4_addr, IPV4_ADDR_DELIM);

            if (byte_vector.size() != IPV4_ADDR_LEN)
            {
                throw std::invalid_argument("IPv4 address must have 4 bytes");
            }

            int index = 0;
            for (auto &byte : byte_vector | std::views::reverse)
            {
                auto byte_int = fmt::stoi(byte);
                if (byte_int < 0 || byte_int > 255)
                {
                    throw std::invalid_argument("IPv4 address byte must be between 0 and 255");
                }
                m_ipv4_addr[index++] = byte_int;
            }
        }

        constexpr uint32_t to_network_order() const
        {
            return array_to_network_order(m_ipv4_addr);
        }

        std::string to_string() const
        {
            std::string ipv4_str;
            for (int i = 0; i < IPV4_ADDR_LEN; i++)
            {
                ipv4_str += fmt::format("{}", m_ipv4_addr[i]);
                if (i != IPV4_ADDR_LEN - 1)
                {
                    ipv4_str += IPV4_ADDR_DELIM;
                }
            }
            return ipv4_str;
        }

        friend std::ostream &operator<<(std::ostream &os, const IPv4 &ipv4)
        {
            os << ipv4.to_string();
            return os;
        }

        constexpr static std::array<ipv4_t, IPV4_ADDR_LEN> from_network_order(const uint32_t &ipv4_addr)
        {
            std::array<ipv4_t, IPV4_ADDR_LEN> ipv4;
            for (int i = 0; i < IPV4_ADDR_LEN; i++)
            {
                ipv4[i] = (ipv4_addr >> (8 * (IPV4_ADDR_LEN - i - 1))) & 0xFF;
            }
            return ipv4;
        }

        constexpr static uint32_t array_to_network_order(const std::array<ipv4_t, IPV4_ADDR_LEN> &ipv4_addr)
        {
            // network order is big-endian
            uint32_t network_order = 0;
            // reverse iterate over the array
            // tested with https://stackoverflow.com/questions/491060/how-to-convert-standard-ip-address-format-string-to-hex-and-long
            // and reverse gotten from https://www.fluentcpp.com/2020/02/11/reverse-for-loops-in-cpp/
            for (const auto &byte : ipv4_addr | std::views::reverse)
            {
                network_order = (network_order << 8) | byte;
            }
            return network_order;
        }

        constexpr bool operator==(const IPv4 &other) const
        {
            return this->m_ipv4_addr == other.m_ipv4_addr;
        }
    };
}