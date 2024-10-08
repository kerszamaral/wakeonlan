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
        constexpr Port() noexcept : Port(0) {}
        constexpr Port(const uint16_t &port) noexcept : m_port(port) {}
        Port(const sockaddr_in &addr) noexcept : m_port(from_network_order(addr.sin_port)) {}

        constexpr port_t getPort() const noexcept
        {
            return m_port;
        }

        port_t to_network_order() const noexcept
        {
            return to_network_order(m_port);
        }

        constexpr void setPort(const port_t &port) noexcept
        {
            m_port = port;
        }

        void setAddrPort(sockaddr_in &addr) const noexcept
        {
            addr.sin_port = this->to_network_order();
        }

        std::string to_string() const noexcept
        {
            return std::to_string(m_port);
        }

        friend std::ostream &operator<<(std::ostream &os, const Port &port) noexcept
        {
            os << port.to_string();
            return os;
        }

        static port_t from_network_order(const port_t &port) noexcept
        {
            return ::ntohs(port);
        }

        static port_t to_network_order(const port_t &port) noexcept
        {
            return ::htons(port);
        }

        constexpr bool operator==(const Port &other) const noexcept
        {
            return m_port == other.m_port;
        }
    };

    constexpr Port MAGIC_PORT = 9;
    constexpr Port DISCOVERY_PORT = 10000;
    constexpr Port EXIT_PORT = 12345;
    constexpr Port MONITOR_PORT = 14321;
    constexpr Port REPLICATION_PORT = 13333;
    constexpr Port ELECTION_PORT = 14444;
}