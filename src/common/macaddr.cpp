#include "common/macaddr.hpp"

#include "common/format.hpp"
#include <stdexcept>
#include <sstream>

MacAddress::MacAddress(std::string mac_addr)
{
    if (mac_addr.size() != 17)
    {
        throw std::invalid_argument("Invalid MAC address format");
    }

    if (mac_addr[2] != ':' || mac_addr[5] != ':' || mac_addr[8] != ':' || mac_addr[11] != ':' || mac_addr[14] != ':')
    {
        throw std::invalid_argument("Invalid MAC address format");
    }

    for (int i = 0; i < MAC_ADDR_LEN; i++)
    {
        const std::string byte = mac_addr.substr(i * 3, 2);
        m_mac_addr[i] = std::stoi(byte, 0, 16);
    }
    // sscanf(mac_addr.c_str(), "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
    //        &m_mac_addr[0], &m_mac_addr[1], &m_mac_addr[2],
    //        &m_mac_addr[3], &m_mac_addr[4], &m_mac_addr[5]);
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
        ss << fmt::string_format("%02x", byte);
        if (&byte != &m_mac_addr.back())
            ss << ":";
    }
    return ss.str();

    // return fmt::string_format("%02x:%02x:%02x:%02x:%02x:%02x", this->m_mac_addr[0], this->m_mac_addr[1], this->m_mac_addr[2], this->m_mac_addr[3], this->m_mac_addr[4], this->m_mac_addr[5]);
}

std::ostream &operator<<(std::ostream &os, const MacAddress &mac)
{
    os << mac.to_string();
    return os;
}

std::size_t std::hash<MacAddress>::operator()(const MacAddress &k) const
{
    std::size_t res = 17;
    for (int i = 0; i < k.MAC_ADDR_LEN; i++)
    {
        res = res * 31 + std::hash<uint8_t>()(k.m_mac_addr[i]);
    }
    return res;
}