#include "constants.hpp"
#include "interface/interface.hpp"
#include <format>

int main()
{
    // pc_map_t pc_map = std::unordered_map<MacAddress, pc_info>();
    for (int i = 0; i < 20; i++)
    {
        pc_info pc = pc_info();
        sprintf(pc.name, "TEST%d", i);
        char temp[18];
        sprintf(temp, "%02x:%02x:%02x:%02x:%02x:%02x", i, i, i, i, i, i);
        std::string mac_addr = std::string(temp);

        // std::cout << mac_addr << std::endl;
        MacAddress mac = MacAddress(mac_addr);
        std::cout << mac << std::endl;
        // pc_map.insert(std::make_pair(MacAddress(mac_addr), pc));
    }

    // print_status(pc_map);

    // while (true)
    // {
    /* code */
    // }
    return 0;
}
