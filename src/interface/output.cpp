#include "interface/output.hpp"
#include "common/pcinfo.hpp"
#include <thread>
#include <iostream>
#include <sstream>

#ifdef _WIN32
constexpr auto CLEAR = "cls";
#else
constexpr auto CLEAR = "clear";
#endif

constexpr const int CHECK_DELAY = 100;

std::string make_pc_table(const pc_map_t &pc_map)
{
    std::stringstream ss;
    ss << "Hostname\tMAC Address\tIPv4 Address\tStatus\n";
    for (auto &pc : pc_map)
    {
        ss << pc.second.get_hostname() << "\t" << pc.second.get_mac().to_string() << "\t" << pc.second.get_ipv4().to_string() << "\t" << std::to_string(pc.second.get_status()) << "\n";
    }
    return ss.str();
}

void WriteCout(const pc_map_t &pc_map, std::atomic<bool> &run, std::atomic<bool> &update)
{
    while (run.load())
    {
        // std::system(CLEAR);
        std::cout << make_pc_table(pc_map) << std::endl;
        while (!update.load() && run.load())
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(CHECK_DELAY));
        }
    }
}