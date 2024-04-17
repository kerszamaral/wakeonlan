#include "subservices/management/exit.hpp"

#include <thread>
#include "networking/sockets/udp.hpp"
#include "threads/signals.hpp"

namespace Subservices::Management::Exit
{
    void sender()
    {
        using namespace Networking;
        using namespace Packets;
        // Wait for program to start shutting down
        Threads::Signals::run.wait(true);
        Sockets::UDP::broadcast(Packet(PacketType::SSE), Addresses::EXIT_PORT);
    }

    void receiver(PC::atomic_pc_map_t &pc_map)
    {
        using namespace Networking;
        constexpr const auto CHECK_DELAY = std::chrono::milliseconds(100);
        auto socket = Sockets::UDP(Addresses::EXIT_PORT);

        while (Threads::Signals::run)
        {
            auto maybe_packet = socket.wait_and_receive_packet(CHECK_DELAY);
            if (!maybe_packet.has_value())
            {
                continue;
            }
            const auto [packet, addr] = maybe_packet.value();
            if (packet.getType() != Packets::PacketType::SSE)
            {
                continue;
            }
            auto hostname = std::get<std::string>(packet.getBody().getPayload());

            auto remove_pc = [&hostname](PC::pc_map_t &pc_map)
            {
                if (pc_map.contains(hostname))
                {
                    auto &pc = pc_map.at(hostname);
                    if (Threads::Signals::manager_found && pc.get_is_manager())
                    {
                        Threads::Signals::manager_found = false;
                        Threads::Signals::manager_found.notify_all();
                    }
                    pc_map.erase(hostname);
                    Threads::Signals::update = true;
                    Threads::Signals::update.notify_all();
                }
            };
            pc_map.execute(remove_pc);
        }
    }
}