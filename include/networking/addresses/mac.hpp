#pragma once

#include <iostream>
#include <cstdint>
#include <array>
#include <sstream>
#include "common/optional.hpp"
#include <cstring>
#include "common/platform.hpp"
#include "common/format.hpp"
#include "networking/sockets/socket.hpp"

namespace Networking::Addresses
{
    constexpr int MAC_ADDR_LEN = 6;
    typedef std::array<uint8_t, MAC_ADDR_LEN> mac_addr_t;
    class Mac
    {
    private:
        constexpr static int MAC_ADDR_STR_LEN = 17;
        constexpr static char MAC_ADDR_DELIM = ':';
        mac_addr_t m_mac_addr;

    public:
        Mac() : Mac("00:00:00:00:00:00") {}
        Mac(const std::string &mac_addr)
        {
            if (mac_addr.size() != MAC_ADDR_STR_LEN)
            {
                throw std::invalid_argument("Invalid MAC address format");
            }

            auto bytes = fmt::split(mac_addr, MAC_ADDR_DELIM);
            if (bytes.size() != MAC_ADDR_LEN)
            {
                throw std::invalid_argument("Invalid MAC address format");
            }

            for (int i = 0; i < MAC_ADDR_LEN; i++)
            {
                if (bytes[i].size() != 2)
                {
                    throw std::invalid_argument("Invalid MAC address format");
                }
                m_mac_addr[i] = std::stoi(bytes[i], 0, 16);
            }
        }
        constexpr Mac(const mac_addr_t &mac_addr) noexcept : m_mac_addr(mac_addr) {}

        static opt::optional<Mac> FromMachine(const std::string &intrfc)
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
                mac_addr_t mac_addr;

                PIP_ADAPTER_ADDRESSES_LH pAddresses = Addresses;
                bool found = false;
                do
                {
                    std::string friendlyName = converter.to_bytes(pAddresses->FriendlyName);
                    if (friendlyName == intrfc)
                    {
                        for (int i = 0; i < MAC_ADDR_LEN; i++)
                        {
                            mac_addr[i] = pAddresses->PhysicalAddress[i];
                        }
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
                // https://gist.github.com/evanslai/3711349
                Sockets::Socket s = Sockets::Socket(Sockets::Type::UDP);
                struct ifreq ifr;
                strncpy(ifr.ifr_name, intrfc.c_str(), IFNAMSIZ);
                auto ioctl_result = ::ioctl(s.getSocket(), SIOCGIFHWADDR, &ifr);
                if (ioctl_result == Sockets::SOCK_ERROR)
                {
                    s.close();
                    return opt::nullopt;
                }
                mac_addr_t mac_addr;
                unsigned char *mac = (unsigned char *)ifr.ifr_hwaddr.sa_data;
                for (int i = 0; i < MAC_ADDR_LEN; i++)
                {
                    mac_addr[i] = mac[i];
                }
                s.close();
#endif
                return Mac(mac_addr);
            }
            catch (const std::exception &e)
            {
                return opt::nullopt;
            }
            return opt::nullopt;
        }

        static Mac FromMachine()
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
                const auto &mac = FromMachine(name);
                if (mac.has_value())
                {
                    return mac.value();
                }
            }
            throw std::runtime_error("Failed to get MAC address from any interface");
        }

        constexpr bool operator==(const Mac &other) const noexcept
        {
            return std::equal(m_mac_addr.begin(), m_mac_addr.end(), other.m_mac_addr.begin());
        }

        constexpr bool operator!=(const Mac &other) const noexcept
        {
            return !(*this == other);
        }

        constexpr bool operator<(const Mac &other) const noexcept
        {
            return std::lexicographical_compare(m_mac_addr.begin(), m_mac_addr.end(), other.m_mac_addr.begin(), other.m_mac_addr.end());
        }

        constexpr const mac_addr_t &data() const noexcept { return m_mac_addr; }

        constexpr size_t size() const noexcept { return m_mac_addr.size(); }
        std::string to_string() const
        {
            std::stringstream ss;
            for (const auto &byte : m_mac_addr)
            {
                ss << fmt::format("%02x", byte);
                if (&byte != &m_mac_addr.back())
                    ss << MAC_ADDR_DELIM;
            }
            return ss.str();
        }

        friend std::ostream &operator<<(std::ostream &os, const Mac &mac)
        {
            os << mac.to_string();
            return os;
        }
    };
}