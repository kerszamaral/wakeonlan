#include "networking/addresses/ipv4.hpp"

#include "common/format.hpp"
#include <stdexcept>
#include <sstream>
#include <algorithm>
#include <string>
#include <iostream>
#include <ranges>

namespace Networking::Addresses
{
    IPv4::IPv4(const std::string &ipv4_addr)
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
        for (auto &byte : byte_vector)
        {
            auto byte_int = std::stoi(byte);
            if (byte_int < 0 || byte_int > 255)
            {
                throw std::invalid_argument("IPv4 address byte must be between 0 and 255");
            }
            m_ipv4_addr[index++] = byte_int;
        }
    }

    IPv4::IPv4(const uint32_t &ipv4_addr)
    {
        for (int i = 0; i < IPV4_ADDR_LEN; i++)
        {
            m_ipv4_addr[i] = (ipv4_addr >> (8 * (IPV4_ADDR_LEN - i - 1))) & 0xFF;
        }
    }

    IPv4::~IPv4()
    {
    }

    uint32_t IPv4::to_network_order() const
    {
        // network order is big-endian
        uint32_t network_order = 0;
        // reverse iterate over the array
        // tested with https://stackoverflow.com/questions/491060/how-to-convert-standard-ip-address-format-string-to-hex-and-long
        // and reverse gotten from https://www.fluentcpp.com/2020/02/11/reverse-for-loops-in-cpp/
        for (const auto &byte : m_ipv4_addr | std::views::reverse)
        {
            network_order = (network_order << 8) | byte;
        }

        return network_order;
    }

    std::string IPv4::to_string() const
    {
        std::stringstream ss;
        for (const auto &byte : m_ipv4_addr)
        {
            ss << (int)byte;
            if (&byte != &m_ipv4_addr.back())
                ss << IPV4_ADDR_DELIM;
        }
        return ss.str();
    }

    std::ostream &operator<<(std::ostream &os, const IPv4 &ipv4)
    {
        os << ipv4.to_string();
        return os;
    }
}