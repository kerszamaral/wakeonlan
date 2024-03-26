#pragma once
#include <iostream>
#include <map>
#include <cstdint>

class MacAddress
{
private:
    uint8_t m_mac_addr[6];

public:
    MacAddress(std::string mac_addr);
    ~MacAddress();

    bool operator==(const MacAddress &other) const
    {
        return std::equal(m_mac_addr, m_mac_addr + 6, other.m_mac_addr);
    }

    bool operator!=(const MacAddress &other) const
    {
        return !(*this == other);
    }

    bool operator<(const MacAddress &other) const
    {
        return std::lexicographical_compare(m_mac_addr, m_mac_addr + 6, other.m_mac_addr, other.m_mac_addr + 6);
    }

    std::string to_string() const
    {
        char mac_addr[18];
        sprintf(mac_addr, "%02x:%02x:%02x:%02x:%02x:%02x", this->m_mac_addr[0], this->m_mac_addr[1], this->m_mac_addr[2], this->m_mac_addr[3], this->m_mac_addr[4], this->m_mac_addr[5]);
        return std::string(mac_addr);
    }

    friend std::ostream &operator<<(std::ostream &os, const MacAddress &mac)
    {
        os << mac.to_string();
        return os;
    }
    friend std::hash<MacAddress>;
};

template <>
struct std::hash<MacAddress>
{
    std::size_t operator()(const MacAddress &k) const
    {
        // Compute individual hash values for mac address
        // http://stackoverflow.com/a/1646913/126995
        std::size_t res = 17;
        for (int i = 0; i < 6; i++)
        {
            res = res * 31 + std::hash<uint8_t>()(k.m_mac_addr[i]);
        }
        return res;
    }
};

typedef struct
{
    char name[20] = "TEST";
} pc_info;

typedef std::map<MacAddress, pc_info> pc_map_t;
