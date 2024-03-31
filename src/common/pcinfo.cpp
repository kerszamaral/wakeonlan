#include "common/pcinfo.hpp"

#include <sstream>

PCInfo::PCInfo(hostname_t hostname, Networking::MacAddress mac, Networking::Addresses::IPv4 ipv4, PC_STATUS status)
    : hostname(hostname), mac(mac), ipv4(ipv4), status(status)
{
}

PCInfo::PCInfo(std::string hostname, std::string mac, std::string ipv4, PC_STATUS status)
    : hostname(hostname), mac(mac), ipv4(ipv4), status(status)
{
}

PCInfo::~PCInfo()
{
}

hostname_t PCInfo::get_hostname() const
{
    return this->hostname;
}

Networking::MacAddress PCInfo::get_mac() const
{
    return this->mac;
}

Networking::Addresses::IPv4 PCInfo::get_ipv4() const
{
    return this->ipv4;
}

PC_STATUS PCInfo::get_status() const
{
    return this->status;
}

void PCInfo::set_status(PC_STATUS status)
{
    this->status = status;
}

void PCInfo::set_ipv4(Networking::Addresses::IPv4 ipv4)
{
    this->ipv4 = ipv4;
}

void PCInfo::set_mac(Networking::MacAddress mac)
{
    this->mac = mac;
}

void PCInfo::set_hostname(hostname_t hostname)
{
    this->hostname = hostname;
}

std::string PCInfo::to_string() const
{
    std::stringstream ss;
    ss << "PCInfo(" << this->hostname << ", " << this->mac << ", " << this->ipv4 << ", " << this->status << ")";
    return ss.str();
}

std::ostream &operator<<(std::ostream &os, const PCInfo &pc)
{
    os << pc.to_string();
    return os;
}