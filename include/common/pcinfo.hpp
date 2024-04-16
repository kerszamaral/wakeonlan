#pragma once

#include <iostream>
#include <map>
#include <cstdint>
#include <string>
#include "networking/addresses/mac.hpp"
#include "networking/addresses/ipv4.hpp"

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
    Networking::Addresses::Mac mac;
    Networking::Addresses::IPv4 ipv4;
    PC_STATUS status;
    bool is_manager;

public:
    PCInfo(hostname_t hostname, Networking::Addresses::Mac mac, Networking::Addresses::IPv4 ipv4, PC_STATUS status, bool is_manager = false);
    PCInfo(std::string hostname, std::string mac, std::string ipv4, PC_STATUS status, bool is_manager = false);
    ~PCInfo();

    // https://stackoverflow.com/questions/70103393/is-there-a-portable-way-in-standard-c-to-retrieve-hostname
    static hostname_t getMachineName();
    hostname_t get_hostname() const;
    Networking::Addresses::Mac get_mac() const;
    Networking::Addresses::IPv4 get_ipv4() const;
    PC_STATUS get_status() const;
    bool get_is_manager() const;

    void set_status(PC_STATUS status);
    void set_ipv4(Networking::Addresses::IPv4 ipv4);
    void set_mac(Networking::Addresses::Mac mac);
    void set_hostname(hostname_t hostname);
    void set_is_manager(bool is_manager);

    std::string to_string() const;

    friend std::ostream &operator<<(std::ostream &os, const PCInfo &pc);
};

typedef std::map<hostname_t, PCInfo> pc_map_t;
