#pragma once

#include <iostream>
#include <map>
#include <cstdint>
#include <string>

#include "threads/atomic.hpp"
#include "threads/queue.hpp"
#include "networking/addresses/mac.hpp"
#include "networking/addresses/ipv4.hpp"

namespace PC
{
    typedef std::string hostname_t;

    enum STATUS
    {
        SLEEPING = 0,
        AWAKE = 1,
        UNKNOWN = 2
    };

    // https://stackoverflow.com/questions/70103393/is-there-a-portable-way-in-standard-c-to-retrieve-hostname
    inline hostname_t getHostname()
    {
        constexpr auto max_hostname_len = 0x100;
        char hostname[max_hostname_len];
        //! On windows, gethostname() needs WSAStartup() to be called first
        if (::gethostname(hostname, sizeof(hostname)) == 0)
        {
            return hostname;
        }
        //! The specification says that if gethostname() fails, we should return the IP address of the machine
        //? But, if we can't get the hostname, we can't get the IP address either
        return "unknown";
    }

    class PCInfo
    {
    private:
        hostname_t hostname;
        Networking::Addresses::Mac mac;
        Networking::Addresses::IPv4 ipv4;
        STATUS status;
        bool is_manager;

    public:
        constexpr PCInfo(hostname_t hostname, Networking::Addresses::Mac mac, Networking::Addresses::IPv4 ipv4, STATUS status, bool is_manager = false)
            : hostname(hostname), mac(mac), ipv4(ipv4), status(status), is_manager(is_manager) {}
        PCInfo(std::string hostname, std::string mac, std::string ipv4, STATUS status, bool is_manager = false)
            : hostname(hostname), mac(mac), ipv4(ipv4), status(status), is_manager(is_manager) {}

        constexpr hostname_t get_hostname() const noexcept { return hostname; }
        constexpr Networking::Addresses::Mac get_mac() const noexcept { return mac; }
        constexpr Networking::Addresses::IPv4 get_ipv4() const noexcept { return ipv4; }
        constexpr STATUS get_status() const noexcept { return status; }
        constexpr bool get_is_manager() const noexcept { return is_manager; }

        constexpr void set_status(STATUS status) noexcept { this->status = status; }
        constexpr void set_ipv4(Networking::Addresses::IPv4 ipv4) noexcept { this->ipv4 = ipv4; }
        constexpr void set_mac(Networking::Addresses::Mac mac) noexcept { this->mac = mac; }
        constexpr void set_hostname(hostname_t hostname) noexcept { this->hostname = hostname; }
        constexpr void set_is_manager(bool is_manager) noexcept { this->is_manager = is_manager; }

        std::string to_string() const
        {
            std::stringstream ss;
            ss << "PCInfo(" << this->hostname << ", " << this->mac << ", " << this->ipv4 << ", " << this->status << ", " << this->is_manager << ")";
            return ss.str();
        }

        friend std::ostream &operator<<(std::ostream &os, const PCInfo &pc)
        {
            os << pc.to_string();
            return os;
        }
    };

    typedef std::map<hostname_t, PCInfo> pc_map_t;
    typedef Threads::Atomic<pc_map_t> atomic_pc_map_t;

    typedef Threads::AtomicQueue<PCInfo> new_pcs_queue;
    typedef Threads::AtomicQueue<hostname_t> wakeups_queue;
    typedef Threads::AtomicQueue<std::pair<hostname_t, STATUS>> sleep_queue;
} // namespace PC
