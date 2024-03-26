#include "constants.hpp"
#include "interface/interface.hpp"
#include <format>
#include <thread>
#include "common/atomic.hpp"

void test0(int &value, int id)
{
    std::cout << "Entrando em Thread " << id << std::endl;
    value++;
    for (int i = 0; i < 1000000; i++)
    {
    }
    std::cout << "Terminando Thread " << id << ", VALUE: " << value << std::endl;
}

void test1(int &value, int id)
{
    std::cout << "Entrando em Thread " << id << std::endl;
    std::cout << "Terminando Thread " << id << ", VALUE: " << value << std::endl;
}

int main()
{
    /*

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
    code
    // }
    */

    auto value = Atomic<int>(10);
    const int num_threads = 2;

    std::thread threads[num_threads];

    threads[0] = std::thread(value.with(), test0, 0);
    threads[1] = std::thread(value.with(), test1, 1);

    std::cout << "Threads criadas com sucesso. Aguardando término..." << std::endl;

    // Aguarda o término de todas as threads
    for (int i = 0; i < num_threads; ++i)
    {
        threads[i].join();
    }

    std::cout << "Todas as threads terminaram." << std::endl;

    return 0;
}
