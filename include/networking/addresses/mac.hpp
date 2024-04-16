#pragma once

#include <iostream>
#include <cstdint>
#include <array>
#include "common/optional.hpp"

namespace Networking::Addresses
{
    constexpr int MAC_ADDR_LEN = 6;
    typedef std::array<uint8_t, MAC_ADDR_LEN> mac_addr_t;
    class Mac
    {
    private:
        constexpr static int MAC_ADDR_STR_LEN = 17;
        constexpr static char MAC_ADDR_DELIM = ':';
        mac_addr_t m_mac_addr;

    public:
        Mac() : Mac("00:00:00:00:00:00") {}
        Mac(std::string mac_addr);
        Mac(const mac_addr_t &mac_addr) : m_mac_addr(mac_addr) {}
        ~Mac();
        static opt::optional<Mac> FromMachine(const std::string &intrfc);
        static opt::optional<Mac> FromMachine();

        bool operator==(const Mac &other) const;

        bool operator!=(const Mac &other) const;

        bool operator<(const Mac &other) const;

        const mac_addr_t &data() const { return m_mac_addr; }

        size_t size() const { return m_mac_addr.size(); }
        std::string to_string() const;

        friend std::ostream &operator<<(std::ostream &os, const Mac &mac);
        friend std::hash<Mac>;
    };
}

template <>
struct std::hash<Networking::Addresses::Mac>
{
    std::size_t operator()(const Networking::Addresses::Mac &k) const;
};
