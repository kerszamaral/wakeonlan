#pragma once

#include <string>
#include <cstdint>
#include <array>

class IPv4
{
private:
    constexpr static char IPV4_ADDR_DELIM = '.';
    constexpr static int IPV4_ADDR_LEN = 4;
    std::array<uint8_t, IPV4_ADDR_LEN> m_ipv4_addr;

public:
    IPv4(std::string ipv4_addr);
    ~IPv4();

    std::string to_string() const;

    friend std::ostream &operator<<(std::ostream &os, const IPv4 &ipv4);
};