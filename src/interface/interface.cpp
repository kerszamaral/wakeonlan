#include "interface/interface.hpp"
#include "interface/input.hpp"
#include "interface/output.hpp"
#include <thread>

void init_interface(Threads::Atomic<pc_map_t> &pc_map, Threads::Signals &signals)
{
    auto inputThread = std::thread(ReadCin, std::ref(signals));
    auto outputThread = std::thread(WriteCout, std::ref(pc_map), std::ref(signals));
    inputThread.join();
    outputThread.join();
}