#include "management/management.hpp"

void init_management(Threads::ProdCosum<PCInfo> &new_pcs, Threads::Atomic<pc_map_t> &pc_map, Threads::Signals &signals)
{
    constexpr const auto CHECK_DELAY = std::chrono::milliseconds(100);
    while (signals.run.load())
    {
        std::this_thread::sleep_for(CHECK_DELAY);
        auto maybe_new_pc = new_pcs.consume();

        if (maybe_new_pc.has_value())
        {
            const auto new_pc = maybe_new_pc.value();
            auto add_pc = [](pc_map_t &pc_map, const PCInfo &new_pc)
            {
                pc_map.emplace(new_pc.get_hostname(), new_pc);
            };
            pc_map.execute(add_pc, new_pc);
            signals.update.store(true);
        }
    }
}