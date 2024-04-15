#pragma once

#include <string>
#include <cstdint>
#include <iostream>

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
        constexpr static port_t MAGIC_PORT = 9;
        constexpr static port_t DISCOVERY_PORT = 10000;
        constexpr static port_t EXIT_PORT = 12345;
        Port() : Port(0) {}
        Port(const uint16_t &port);
        Port(const std::string &port);
        ~Port();

        port_t getPort() const;

        std::string to_string() const;

        friend std::ostream &operator<<(std::ostream &os, const Port &port);

        bool operator==(const Port &other) const;
    };
}