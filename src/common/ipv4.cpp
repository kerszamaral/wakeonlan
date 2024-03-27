#include "common/ipv4.hpp"

#include "common/format.hpp"
#include <stdexcept>
#include <sstream>
#include <algorithm>
#include <string>
#include <iostream>
#include <ranges>

IPv4::IPv4(std::string ipv4_addr)
{
    if (ipv4_addr.empty())
    {
        throw std::invalid_argument("IPv4 address cannot be empty");
    }

    auto byte_vector = fmt::string_split(ipv4_addr, '.');

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

IPv4::~IPv4()
{
}

std::string IPv4::to_string() const
{
    std::stringstream ss;
    for (const auto &byte : m_ipv4_addr)
    {
        ss << (int)byte;
        if (&byte != &m_ipv4_addr.back())
            ss << ".";
    }
    return ss.str();
}

std::ostream &operator<<(std::ostream &os, const IPv4 &ipv4)
{
    os << ipv4.to_string();
    return os;
}