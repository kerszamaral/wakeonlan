#pragma once

#include <cstring>
#include <cstdint>
#include <array>
#include <algorithm>
#include <ranges>
#include <sstream>
#include "common/platform.hpp"
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

        static opt::optional<IPv4> FromMachine(const std::string &intrfc )
        {
            try
            {
#ifdef OS_WIN
                IP_ADAPTER_ADDRESSES_LH Addresses[16];
                ULONG outBufLen = sizeof(Addresses);
                ULONG flags = GAA_FLAG_INCLUDE_PREFIX;

                auto dwStatus = GetAdaptersAddresses(AF_INET, flags, NULL, Addresses, &outBufLen);
                if (dwStatus != 0)
                {
                    return opt::nullopt;
                }

                using convert_type = std::codecvt_utf8<wchar_t>;
                std::wstring_convert<convert_type, wchar_t> converter;
                uint32_t ip;

                PIP_ADAPTER_ADDRESSES_LH pAddresses = Addresses;
                bool found = false;
                do
                {
                    std::string friendlyName = converter.to_bytes(pAddresses->FriendlyName);
                    if (friendlyName == intrfc)
                    {
                        ip = ((struct sockaddr_in *)pAddresses->FirstUnicastAddress->Address.lpSockaddr)->sin_addr.S_un.S_addr;
                        found = true;
                        break;
                    }

                    pAddresses = pAddresses->Next;
                } while (pAddresses);

                if (!found)
                {
                    return opt::nullopt;
                }
#else
                int fd = socket(AF_INET, SOCK_DGRAM, 0);
                struct ifreq ifr;
                strncpy(ifr.ifr_name, intrfc.c_str(), IFNAMSIZ);
                auto ioctl_result = ::ioctl(fd, SIOCGIFADDR, &ifr);
                if (ioctl_result == -1)
                {
                    close(fd);
                    return opt::nullopt;
                }
                uint32_t ip = ((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr.s_addr;
                close(fd);
#endif
                return IPv4(ip);
            }
            catch (const std::exception &e)
            {
                return opt::nullopt;
            }
            return opt::nullopt;
        }

        static IPv4 FromMachine()
        {
#ifdef OS_WIN
            const auto &names = {
                "Ethernet",
            };
#else
            const auto &names = {
                "eth0",
                //! The pc from my lab has a weird issues with the name of the interface
                "enp4s0",
                "ether",
            };
#endif
            for (const auto &name : names)
            {
                const auto &ip = FromMachine(name);
                if (ip.has_value())
                {
                    return ip.value();
                }
            }
            throw std::runtime_error("Failed to get IPv4 address from any interface");
        }
    };

    const IPv4 BROADCAST_IP = {"255.255.255.255"};
}