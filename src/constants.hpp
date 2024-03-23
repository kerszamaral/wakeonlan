#pragma once
#include <iostream>
#include <unordered_map>
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
};

typedef struct
{
    char name[20] = "TEST";
} pc_info;

typedef std::unordered_map<MacAddress, pc_info> pc_map_t;

typedef struct
{
    uint16_t type;        // Tipo do pacote (p.ex. DATA | CMD)
    uint16_t seqn;        // Número de sequência
    uint16_t length;      // Comprimento do payload
    uint16_t timestamp;   // Timestamp do dado
    const char *_payload; // Dados da mensagem
} packet;
