#pragma once

#include <string>
#include <cstdint>
#include <iostream>
#include <algorithm>

#include "common/format.hpp"

namespace Networking::Addresses
{
    typedef uint16_t port_t;

    class Port
    {
    private:
        constexpr static port_t PORT_MIN = 0;
        constexpr static port_t PORT_MAX = 65535;
        port_t m_port;

    public:
        constexpr Port() : Port(0) {}
        constexpr Port(const uint16_t &port) : m_port(port) {}

        constexpr port_t getPort() const
        {
            return m_port;
        }

        std::string to_string() const
        {
            return std::to_string(m_port);
        }

        friend std::ostream &operator<<(std::ostream &os, const Port &port)
        {
            os << port.to_string();
            return os;
        }

        constexpr bool operator==(const Port &other) const
        {
            return m_port == other.m_port;
        }
    };

    constexpr Port MAGIC_PORT = 9;
    constexpr Port DISCOVERY_PORT = 10000;
    constexpr Port EXIT_PORT = 12345;
    constexpr Port MONITOR_PORT = 14321;
}