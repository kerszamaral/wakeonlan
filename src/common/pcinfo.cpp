#include "common/pcinfo.hpp"

#include <sstream>
#include "common/platform.hpp"

namespace PC
{
    // https://stackoverflow.com/questions/70103393/is-there-a-portable-way-in-standard-c-to-retrieve-hostname
    hostname_t getHostname()
    {
        std::string res = "unknown";
        char tmp[0x100];
        //! On windows, gethostname() needs WSAStartup() to be called first
        if (gethostname(tmp, sizeof(tmp)) == 0)
        {
            res = tmp;
        }
        return res;
    }

    PCInfo::PCInfo(hostname_t hostname, Networking::Addresses::Mac mac, Networking::Addresses::IPv4 ipv4, STATUS status, bool is_manager)
        : hostname(hostname), mac(mac), ipv4(ipv4), status(status), is_manager(is_manager)
    {
    }

    PCInfo::PCInfo(std::string hostname, std::string mac, std::string ipv4, STATUS status, bool is_manager)
        : hostname(hostname), mac(mac), ipv4(ipv4), status(status), is_manager(is_manager)
    {
    }

    PCInfo::~PCInfo()
    {
    }

    hostname_t PCInfo::get_hostname() const
    {
        return this->hostname;
    }

    Networking::Addresses::Mac PCInfo::get_mac() const
    {
        return this->mac;
    }

    Networking::Addresses::IPv4 PCInfo::get_ipv4() const
    {
        return this->ipv4;
    }

    STATUS PCInfo::get_status() const
    {
        return this->status;
    }

    bool PCInfo::get_is_manager() const
    {
        return this->is_manager;
    }

    void PCInfo::set_status(STATUS status)
    {
        this->status = status;
    }

    void PCInfo::set_ipv4(Networking::Addresses::IPv4 ipv4)
    {
        this->ipv4 = ipv4;
    }

    void PCInfo::set_mac(Networking::Addresses::Mac mac)
    {
        this->mac = mac;
    }

    void PCInfo::set_hostname(hostname_t hostname)
    {
        this->hostname = hostname;
    }

    void PCInfo::set_is_manager(bool is_manager)
    {
        this->is_manager = is_manager;
    }

    std::string PCInfo::to_string() const
    {
        std::stringstream ss;
        ss << "PCInfo(" << this->hostname << ", " << this->mac << ", " << this->ipv4 << ", " << this->status << ", " << this->is_manager << ")";
        return ss.str();
    }

    std::ostream &operator<<(std::ostream &os, const PCInfo &pc)
    {
        os << pc.to_string();
        return os;
    }
}