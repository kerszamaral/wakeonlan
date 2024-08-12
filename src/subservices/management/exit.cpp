#include "subservices/management/exit.hpp"

#include <thread>
#include "networking/sockets/udp.hpp"
#include "threads/signals.hpp"
#include "threads/delays.hpp"

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

    void remove_pc(PC::pc_map_t &pc_map, const PC::hostname_t &hostname, PC::updates_queue &updates)
    {
        if (pc_map.contains(hostname))
        {
            auto &pc = pc_map.at(hostname);
            if (Threads::Signals::current_manager != 0 && pc.get_is_manager())
            {
                Threads::Signals::current_manager = 0;
                Threads::Signals::current_manager.notify_all();
                Threads::Signals::force_election = true;
            }
            pc_map.erase(hostname);
            updates.produce(std::make_pair(PC::UPDATE_TYPE::REMOVE, pc));
            Threads::Signals::update = true;
            Threads::Signals::update.notify_all();
        }
    }

    void receiver(PC::atomic_pc_map_t &pc_map, PC::updates_queue &updates)
    {
        using namespace Networking;
        auto socket = Sockets::UDP(Addresses::EXIT_PORT);

        while (Threads::Signals::run)
        {
            auto maybe_packet = socket.wait_and_receive_packet(Threads::Delays::CHECK_DELAY);
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
            pc_map.execute(remove_pc, hostname, updates);
            std::this_thread::sleep_for(Threads::Delays::WAIT_DELAY);
        }
    }
}