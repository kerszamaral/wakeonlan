#include "interface/output.hpp"

#include <thread>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <cstring>
#include "common/platform.hpp"
#include "common/pcinfo.hpp"

std::string make_pc_table(const pc_map_t &pc_map, const std::atomic<bool> &is_manager)
{
    std::stringstream ss;
    constexpr const auto MANAGER_TAG = " *";

    constexpr const auto HOSTNAME_HEADER = "Hostname";
    constexpr const auto HOSTNAME_S_SIZE = 20;
    const auto hostname_header = std::string(HOSTNAME_HEADER) + (is_manager.load() ? MANAGER_TAG : "");
    ss << std::left;
    ss << std::setw(HOSTNAME_S_SIZE) << hostname_header.c_str();

    constexpr const auto MAC_HEADER = "MAC Address";
    constexpr const auto MAC_S_SIZE = 20;
    ss << std::setw(MAC_S_SIZE) << MAC_HEADER;

    constexpr const auto IPV4_HEADER = "IPv4 Address";
    constexpr const auto IP_S_SIZE = 17;
    ss << std::setw(IP_S_SIZE) << IPV4_HEADER;

    constexpr const auto STATUS_HEADER = "Status";
    constexpr const auto STATUS_S_SIZE = 8;
    ss << std::setw(STATUS_S_SIZE) << STATUS_HEADER << std::endl;

    for (auto &pc : pc_map)
    {
        const auto hostname = pc.second.get_hostname() + (pc.second.get_is_manager() ? MANAGER_TAG : "");
        ss << std::setw(HOSTNAME_S_SIZE) << hostname.c_str();

        const auto &mac = pc.second.get_mac().to_string();
        ss << std::setw(MAC_S_SIZE) << mac.c_str();

        const auto &ipv4 = pc.second.get_ipv4().to_string();
        ss << std::setw(IP_S_SIZE) << ipv4.c_str();

        const auto &status = pc.second.get_status() == PC_STATUS::AWAKE ? "Awake" : "Asleep";
        ss << std::setw(STATUS_S_SIZE) << status << std::endl;
    }
    return ss.str();
}

#ifdef OS_WIN
constexpr auto CLEAR = "cls";
#else
constexpr auto CLEAR = "clear";
#endif

void WriteCout(Threads::Atomic<pc_map_t> &pc_map, Threads::Signals &signals)
{
    constexpr const auto CHECK_DELAY = std::chrono::milliseconds(100);
    std::string table = pc_map.compute(make_pc_table, signals.is_manager);
    while (signals.run.load())
    {
#ifndef DEBUG
        std::system(CLEAR);
#endif
        std::cout << table << std::endl;

        while (!signals.update.load() && signals.run.load())
        {
            std::this_thread::sleep_for(CHECK_DELAY);
        }
        table = pc_map.compute(make_pc_table, signals.is_manager);
        signals.update.store(false);
    }
}