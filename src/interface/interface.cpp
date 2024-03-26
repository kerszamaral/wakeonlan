#include "interface/interface.hpp"

void print_status(const pc_map_t &pc_map)
{
    // while (true)
    {
        for (auto &pc : pc_map)
        {
            std::cout << "MAC: " << pc.first.to_string() << " | NAME: " << pc.second.name << std::endl;
        }
    }
}