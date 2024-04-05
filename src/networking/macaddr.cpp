#include "networking/macaddr.hpp"

#include "common/format.hpp"
#include <stdexcept>
#include <sstream>

namespace Networking
{
    MacAddress::MacAddress(std::string mac_addr)
    {
        if (mac_addr.size() != MAC_ADDR_STR_LEN)
        {
            throw std::invalid_argument("Invalid MAC address format");
        }

        auto bytes = fmt::split(mac_addr, MAC_ADDR_DELIM);
        if (bytes.size() != MAC_ADDR_LEN)
        {
            throw std::invalid_argument("Invalid MAC address format");
        }

        for (int i = 0; i < MAC_ADDR_LEN; i++)
        {
            if (bytes[i].size() != 2)
            {
                throw std::invalid_argument("Invalid MAC address format");
            }
            m_mac_addr[i] = std::stoi(bytes[i], 0, 16);
        }
    }

    MacAddress::~MacAddress()
    {
    }

    bool MacAddress::operator==(const MacAddress &other) const
    {
        return std::equal(m_mac_addr.begin(), m_mac_addr.end(), other.m_mac_addr.begin());
    }

    bool MacAddress::operator!=(const MacAddress &other) const
    {
        return !(*this == other);
    }

    bool MacAddress::operator<(const MacAddress &other) const
    {
        return std::lexicographical_compare(m_mac_addr.begin(), m_mac_addr.end(), other.m_mac_addr.begin(), other.m_mac_addr.end());
    }

    std::string MacAddress::to_string() const
    {
        std::stringstream ss;
        for (const auto &byte : m_mac_addr)
        {
            ss << fmt::format("%02x", byte);
            if (&byte != &m_mac_addr.back())
                ss << MAC_ADDR_DELIM;
        }
        return ss.str();
    }

    std::ostream &operator<<(std::ostream &os, const MacAddress &mac)
    {
        os << mac.to_string();
        return os;
    }
}

std::size_t std::hash<Networking::MacAddress>::operator()(const Networking::MacAddress &k) const
{
    std::size_t res = 17;
    for (int i = 0; i < Networking::MAC_ADDR_LEN; i++)
    {
        res = res * 31 + std::hash<uint8_t>()(k.m_mac_addr[i]);
    }
    return res;
}