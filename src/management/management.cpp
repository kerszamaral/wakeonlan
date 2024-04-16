#include "management/management.hpp"

#include <thread>
#include "networking/sockets/udp.hpp"
#include "threads/signals.hpp"

void update_pc_map(Threads::ProdCosum<PCInfo> &new_pcs, Threads::Atomic<pc_map_t> &pc_map);

void send_wakeup(Threads::ProdCosum<hostname_t> &wakeups, Threads::Atomic<pc_map_t> &pc_map);

void send_exit();

void exit_receiver(Threads::Atomic<pc_map_t> &pc_map);

void init_management(Threads::ProdCosum<PCInfo> &new_pcs, Threads::Atomic<pc_map_t> &pc_map, Threads::ProdCosum<hostname_t> &wakeups)
{
    {
        std::vector<std::jthread> subservices;
        subservices.emplace_back(update_pc_map, std::ref(new_pcs), std::ref(pc_map));
        subservices.emplace_back(send_wakeup, std::ref(wakeups), std::ref(pc_map));
        subservices.emplace_back(exit_receiver, std::ref(pc_map));
        subservices.emplace_back(send_exit);
    }
}

void update_pc_map(Threads::ProdCosum<PCInfo> &new_pcs, Threads::Atomic<pc_map_t> &pc_map)
{
    constexpr const auto CHECK_DELAY = std::chrono::milliseconds(100);
    while (Threads::Signals::run)
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
            Threads::Signals::update = true;
        }
    }
}

namespace Sockets = Networking::Sockets;

void send_wakeup(Threads::ProdCosum<hostname_t> &wakeups, Threads::Atomic<pc_map_t> &pc_map)
{
    while (Threads::Signals::run)
    {
        auto maybe_wakeup = wakeups.consume();
        if (maybe_wakeup.has_value())
        {
            const auto wakeup = maybe_wakeup.value();
            auto wakeup_pc = [&wakeup](pc_map_t &pc_map)
            {
                if (pc_map.contains(wakeup))
                {
                    const auto &pc = pc_map.at(wakeup);
                    if (pc.get_status() == PC_STATUS::SLEEPING)
                    {
                        Sockets::UDP::broadcast_wakeup(pc.get_mac());
                        std::cout << "Waking up " << wakeup << std::endl;
                    }
                    else
                    {
                        std::cout << wakeup << " is not sleeping" << std::endl;
                    }
                }
                else
                {
                    std::cout << "PC not found" << std::endl;
                }
            };
            pc_map.execute(wakeup_pc);
        }
    }
}

void send_exit()
{
    using namespace Networking;
    Sockets::UDP socket = Sockets::UDP();
    const auto hostname = PCInfo::getMachineName();
    const auto exit_packet = Packet(PacketType::SSE, hostname);
    // Wait for program to start shutting down
    while (Threads::Signals::run)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    Sockets::UDP::broadcast(exit_packet, Addresses::Port::EXIT_PORT);
}

void exit_receiver(Threads::Atomic<pc_map_t> &pc_map)
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

        auto remove_pc = [&hostname](pc_map_t &pc_map)
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