#include "interface/interface.hpp"
#include "interface/input.hpp"
#include "interface/output.hpp"
#include <thread>

void init_interface(const pc_map_t &pc_map, std::atomic<bool> &run, std::atomic<bool> &update)
{
    auto inputThread = std::thread(ReadCin, std::ref(run));
    auto outputThread = std::thread(WriteCout, std::ref(pc_map), std::ref(run), std::ref(update));
    inputThread.join();
    outputThread.join();
}