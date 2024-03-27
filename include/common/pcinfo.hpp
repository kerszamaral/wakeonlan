#pragma once

#include <iostream>
#include <map>
#include <cstdint>
#include "networking/macaddr.hpp"
#include "networking/ipv4.hpp"

typedef std::string hostname_t;

enum PC_STATUS
{
    SLEEPING = 0,
    AWAKE = 1,
    UNKNOWN = 2
};

class PCInfo
{
private:
    hostname_t hostname;
    MacAddress mac;
    IPv4 ipv4;
    PC_STATUS status;

public:
    PCInfo(hostname_t hostname, MacAddress mac, IPv4 ipv4, PC_STATUS status);
    PCInfo(std::string hostname, std::string mac, std::string ipv4, PC_STATUS status);
    ~PCInfo();

    hostname_t get_hostname() const;
    MacAddress get_mac() const;
    IPv4 get_ipv4() const;
    PC_STATUS get_status() const;

    void set_status(PC_STATUS status);
    void set_ipv4(IPv4 ipv4);
    void set_mac(MacAddress mac);
    void set_hostname(hostname_t hostname);

    std::string to_string() const;

    friend std::ostream &operator<<(std::ostream &os, const PCInfo &pc);
};

typedef std::map<hostname_t, PCInfo> pc_map_t;
