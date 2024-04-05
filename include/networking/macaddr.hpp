#pragma once

#include <iostream>
#include <cstdint>
#include <array>

namespace Networking
{
    constexpr int MAC_ADDR_LEN = 6;
    typedef std::array<uint8_t, MAC_ADDR_LEN> mac_addr_t;
    class MacAddress
    {
    private:
        constexpr static int MAC_ADDR_STR_LEN = 17;
        constexpr static char MAC_ADDR_DELIM = ':';
        mac_addr_t m_mac_addr;

    public:
        MacAddress() : MacAddress("00:00:00:00:00:00") {}
        MacAddress(std::string mac_addr);
        MacAddress(const mac_addr_t &mac_addr) : m_mac_addr(mac_addr) {}
        ~MacAddress();

        bool operator==(const MacAddress &other) const;

        bool operator!=(const MacAddress &other) const;

        bool operator<(const MacAddress &other) const;

        const mac_addr_t &data() const { return m_mac_addr; }

        size_t size() const { return m_mac_addr.size(); }
        std::string to_string() const;

        friend std::ostream &operator<<(std::ostream &os, const MacAddress &mac);
        friend std::hash<MacAddress>;
    };
}

template <>
struct std::hash<Networking::MacAddress>
{
    std::size_t operator()(const Networking::MacAddress &k) const;
};
