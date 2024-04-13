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

void WriteCout(Threads::Atomic<pc_map_t> &pc_map, Threads::Signals &signals)
{
    constexpr const auto CHECK_DELAY = std::chrono::milliseconds(100);
    std::string table = pc_map.compute(make_pc_table);
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
        table = pc_map.compute(make_pc_table);
        signals.update.store(false);
    }
}