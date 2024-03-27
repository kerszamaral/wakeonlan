#pragma once

#include <string>
#include <cstdint>
#include <iostream>

class Port
{
private:
    constexpr static uint16_t PORT_MIN = 0;
    constexpr static uint16_t PORT_MAX = 65535;
    uint16_t m_port;

public:
    Port(uint16_t port);
    Port(std::string port);
    ~Port();

    uint16_t get_port() const;

    friend std::ostream &operator<<(std::ostream &os, const Port &port);
};