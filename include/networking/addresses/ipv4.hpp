#pragma once

#include <string>
#include <cstdint>
#include <array>
#include <algorithm>
#include <ranges>
#include <sstream>
#include "common/format.hpp"

namespace Networking::Addresses
{
    constexpr size_t IPV4_ADDR_LEN = 4;
    typedef union ipv4_t
    {
        uint32_t addr;
        std::array<uint8_t, IPV4_ADDR_LEN> bytes;
    } ipv4_t;

    class IPv4
    {
    private:
        constexpr static char IPV4_ADDR_DELIM = '.';
        ipv4_t m_ipv4_addr;

    public:
        constexpr IPv4() noexcept : IPv4(0) {}
        constexpr IPv4(const std::array<uint8_t, IPV4_ADDR_LEN> &ipv4_addr) noexcept
        {
            m_ipv4_addr.bytes = ipv4_addr;
        }
        constexpr IPv4(const uint32_t &ipv4_addr) noexcept
        {
            m_ipv4_addr.addr = ipv4_addr;
        }
        constexpr IPv4(const sockaddr_in &addr) noexcept
        {
#ifdef OS_WIN
            m_ipv4_addr.addr = addr.sin_addr.S_un.S_addr;
#else
            m_ipv4_addr.addr = addr.sin_addr.s_addr;
#endif
        }
        IPv4(const std::string &ipv4_addr)
        {
            if (ipv4_addr.empty())
            {
                throw std::invalid_argument("IPv4 address cannot be empty");
            }

            auto byte_vector = fmt::split(ipv4_addr, IPV4_ADDR_DELIM);

            if (byte_vector.size() != IPV4_ADDR_LEN)
            {
                throw std::invalid_argument("IPv4 address must have 4 bytes");
            }

            //! Check if the ordering is correct
            int index = 0;
            for (auto &byte : byte_vector)
            {
                auto byte_int = fmt::stoi(byte);
                if (byte_int < 0 || byte_int > 255)
                {
                    throw std::invalid_argument("IPv4 address byte must be between 0 and 255");
                }
                m_ipv4_addr.bytes[index++] = byte_int;
            }
        }

        constexpr uint32_t to_network_order() const noexcept
        {
            return m_ipv4_addr.addr;
        }

        constexpr void to_addr(sockaddr_in &addr) noexcept
        {
#ifdef OS_WIN
            addr.sin_addr.S_un.S_addr = m_ipv4_addr.addr;
#else
            addr.sin_addr.s_addr = m_ipv4_addr.addr;
#endif
        }

        std::string to_string() const
        {
            std::stringstream ss;
            for (auto &byte : m_ipv4_addr.bytes)
            {
                ss << fmt::format("%d", byte);
                if (&byte != &m_ipv4_addr.bytes.back())
                    ss << IPV4_ADDR_DELIM;
            }
            return ss.str();
        }

        friend std::ostream &operator<<(std::ostream &os, const IPv4 &ipv4)
        {
            os << ipv4.to_string();
            return os;
        }

        constexpr bool operator==(const IPv4 &other) const noexcept
        {
            return this->m_ipv4_addr.addr == other.m_ipv4_addr.addr;
        }
    };

    const IPv4 BROADCAST_IP = {"255.255.255.255"};
}