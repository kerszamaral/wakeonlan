#include "interface/output.hpp"
#include "common/constants.hpp"
#include <thread>
#include <iostream>

#ifdef _WIN32
constexpr auto CLEAR = "cls";
#else
constexpr auto CLEAR = "clear";
#endif

constexpr const int CHECK_DELAY = 100;

std::string make_pc_table(const pc_map_t &pc_map)
{
    std::string table = "MAC\t\tNAME\n";
    for (auto &pc : pc_map)
    {
        table += pc.first.to_string() + "\t" + pc.second.name + "\n";
    }
    return table;
}

void WriteCout(const pc_map_t &pc_map, std::atomic<bool> &run, std::atomic<bool> &update)
{
    while (run.load())
    {
        std::system(CLEAR);
        std::cout << make_pc_table(pc_map) << std::endl;
        while (!update.load() && run.load())
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(CHECK_DELAY));
        }
    }
}