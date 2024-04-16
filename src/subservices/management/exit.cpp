#include "subservices/management/exit.hpp"

#include <thread>
#include "networking/sockets/udp.hpp"
#include "threads/signals.hpp"

namespace Subservices::Management::Exit
{
    void sender()
    {
        using namespace Networking;
        Sockets::UDP socket = Sockets::UDP();
        const auto hostname = PC::getHostname();
        const auto exit_packet = Packet(PacketType::SSE, hostname);
        // Wait for program to start shutting down
        while (Threads::Signals::run)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        Sockets::UDP::broadcast(exit_packet, Addresses::Port::EXIT_PORT);
    }

    void receiver(PC::atomic_pc_map_t &pc_map)
    {
        using namespace Networking;
        constexpr const auto CHECK_DELAY = std::chrono::milliseconds(100);
        const auto exit_port = Addresses::Port(Addresses::Port::EXIT_PORT);
        auto socket = Sockets::UDP(exit_port);
        while (Threads::Signals::run)
        {
            auto maybe_packet = socket.wait_and_receive_packet(CHECK_DELAY);
            if (!maybe_packet.has_value())
            {
                continue;
            }
            const auto [packet, addr] = maybe_packet.value();
            if (packet.getType() != PacketType::SSE)
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
                    }
                    pc_map.erase(hostname);
                    Threads::Signals::update = true;
                }
            };
            pc_map.execute(remove_pc);
        }
    }
}