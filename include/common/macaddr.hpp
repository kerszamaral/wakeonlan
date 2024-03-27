#pragma once

#include <iostream>
#include <cstdint>
#include <array>

class MacAddress
{
private:
    constexpr static int MAC_ADDR_STR_LEN = 17;
    constexpr static char MAC_ADDR_DELIM = ':';
    constexpr static int MAC_ADDR_LEN = 6;
    std::array<uint8_t, MAC_ADDR_LEN> m_mac_addr;

public:
    MacAddress(std::string mac_addr);
    ~MacAddress();

    bool operator==(const MacAddress &other) const;

    bool operator!=(const MacAddress &other) const;

    bool operator<(const MacAddress &other) const;

    std::string to_string() const;

    friend std::ostream &operator<<(std::ostream &os, const MacAddress &mac);
    friend std::hash<MacAddress>;
};

template <>
struct std::hash<MacAddress>
{
    std::size_t operator()(const MacAddress &k) const;
};