#include "management/management.hpp"

#include <thread>
#include "networking/sockets/udp.hpp"

void update_pc_map(Threads::ProdCosum<PCInfo> &new_pcs, Threads::Atomic<pc_map_t> &pc_map, Threads::Signals &signals);

void send_wakeup(Threads::ProdCosum<hostname_t> &wakeups, Threads::Atomic<pc_map_t> &pc_map, Threads::Signals &signals);

void send_exit(Threads::Signals &signals);

void exit_receiver(Threads::Atomic<pc_map_t> &pc_map, Threads::Signals &signals);

void init_management(Threads::ProdCosum<PCInfo> &new_pcs, Threads::Atomic<pc_map_t> &pc_map, Threads::ProdCosum<hostname_t> &wakeups, Threads::Signals &signals)
{
    {
        std::vector<std::jthread> subservices;
        subservices.emplace_back(update_pc_map, std::ref(new_pcs), std::ref(pc_map), std::ref(signals));
        subservices.emplace_back(send_wakeup, std::ref(wakeups), std::ref(pc_map), std::ref(signals));
        subservices.emplace_back(exit_receiver, std::ref(pc_map), std::ref(signals));
        subservices.emplace_back(send_exit, std::ref(signals));
    }
}

void update_pc_map(Threads::ProdCosum<PCInfo> &new_pcs, Threads::Atomic<pc_map_t> &pc_map, Threads::Signals &signals)
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

namespace Sockets = Networking::Sockets;

void send_wakeup(Threads::ProdCosum<hostname_t> &wakeups, Threads::Atomic<pc_map_t> &pc_map, Threads::Signals &signals)
{
    while (signals.run.load())
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
constexpr const uint16_t EXIT_PORT = 12345;
void send_exit(Threads::Signals &signals)
{
    using namespace Networking;
    Sockets::UDP socket = Sockets::UDP();
    const auto hostname = PCInfo::getMachineName();
    const auto exit_packet = Packet(PacketType::SSE, hostname);
    // Wait for program to start shutting down
    while (signals.run.load())
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    Sockets::UDP::broadcast(exit_packet, Addresses::Port::EXIT_PORT);
}

void exit_receiver(Threads::Atomic<pc_map_t> &pc_map, Threads::Signals &signals)
{
    using namespace Networking;
    constexpr const auto CHECK_DELAY = std::chrono::milliseconds(100);
    const auto exit_port = Addresses::Port(Addresses::Port::EXIT_PORT);
    auto socket = Sockets::UDP(exit_port);
    while (signals.run.load())
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
#ifdef DEBUG
        std::cout << hostname << " is shutting down" << std::endl;
#endif
        auto remove_pc = [&hostname](pc_map_t &pc_map)
        {
            if (pc_map.contains(hostname))
            {
                pc_map.erase(hostname);
            }
        };
        pc_map.execute(remove_pc);
    }
}