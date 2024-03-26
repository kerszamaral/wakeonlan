#include "common/constants.hpp"
#include <string>

MacAddress::MacAddress(std::string mac_addr)
{
    if (mac_addr.length() != 17)
    {
        throw std::invalid_argument("Invalid MAC address");
    }
    if (mac_addr[2] != ':' || mac_addr[5] != ':' || mac_addr[8] != ':' || mac_addr[11] != ':' || mac_addr[14] != ':')
    {
        throw std::invalid_argument("Invalid MAC address");
    }

    for (int i = 0; i < 6; i++)
    {
        // 00:00:00:00:00:00
        const std::string byte = mac_addr.substr(i * 3, 2);
        m_mac_addr[i] = std::stoi(byte, 0, 16);
    }
}

MacAddress::~MacAddress()
{
}