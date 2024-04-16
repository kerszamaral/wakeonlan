#include "networking/addresses/mac.hpp"

#include "common/format.hpp"
#include "networking/sockets/socket.hpp"
#include <stdexcept>
#include <sstream>
#include <cstring>

namespace Networking::Addresses
{
    Mac::Mac(std::string mac_addr)
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

    Mac::~Mac()
    {
    }

    bool Mac::operator==(const Mac &other) const
    {
        return std::equal(m_mac_addr.begin(), m_mac_addr.end(), other.m_mac_addr.begin());
    }

    bool Mac::operator!=(const Mac &other) const
    {
        return !(*this == other);
    }

    bool Mac::operator<(const Mac &other) const
    {
        return std::lexicographical_compare(m_mac_addr.begin(), m_mac_addr.end(), other.m_mac_addr.begin(), other.m_mac_addr.end());
    }

    std::string Mac::to_string() const
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

    std::ostream &operator<<(std::ostream &os, const Mac &mac)
    {
        os << mac.to_string();
        return os;
    }

    opt::optional<Mac> Mac::FromMachine()
    {
#ifdef OS_WIN
        std::string name = "Ethernet";
#else
        std::string name = "eth0";
#endif
        return FromMachine(name);
    }

    opt::optional<Mac> Mac::FromMachine(const std::string &intrfc)
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
                return std::nullopt;
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
                return std::nullopt;
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
                return std::nullopt;
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
            return std::nullopt;
        }
        return std::nullopt;
    }
}

std::size_t std::hash<Networking::Addresses::Mac>::operator()(const Networking::Addresses::Mac &k) const
{
    std::size_t res = 17;
    for (int i = 0; i < Networking::Addresses::MAC_ADDR_LEN; i++)
    {
        res = res * 31 + std::hash<uint8_t>()(k.m_mac_addr[i]);
    }
    return res;
}