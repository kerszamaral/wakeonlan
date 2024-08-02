#include "subservices/election/election.hpp"

#include <thread>

#include "threads/delays.hpp"
#include "threads/signals.hpp"
#include "networking/sockets/udp.hpp"
#include "networking/packets/util.hpp"

namespace Subservices::Election
{
    using namespace Networking;
    bool elected(Sockets::UDP &conn, const Addresses::IPv4 &our_ip)
    {
        const uint32_t our_number = Threads::Signals::table_version;
        bool someone_is_greater = false;
        constexpr uint32_t MAX_TURNS = 5;
        uint32_t turns_left = MAX_TURNS;

        // Packets
        const auto greater_packet = Packets::Packet(Packets::PacketType::SSELGT);
        const auto election_packet = Packets::Packet(Packets::PacketType::SSEL, our_number);

        while (Threads::Signals::run && turns_left > 0)
        {
            // We check if someone is greater than us
            if (!someone_is_greater)
            {
                // We send our number to the network
                conn.send_broadcast(election_packet, Addresses::ELECTION_PORT);
            }
            auto current_turn = turns_left;
            while (Threads::Signals::run && current_turn == turns_left)
            {
                auto resp = conn.wait_and_receive_packet(Threads::Delays::WAIT_DELAY);
                if (!resp.has_value())
                {
                    // No response,
                    // We decrement the turn counter
                    turns_left--;
                    continue;
                }
                auto [packet, addr] = resp.value();
                if (packet.getType() == Packets::PacketType::SSELFIN)
                {
                    // Election is finished, we wait to find manager
                    // on another thread
                    return false; // Exit election
                }
                else if (packet.getType() == Packets::PacketType::SSEL)
                {
                    // Election is still going on
                    const auto their_number = std::get<uint32_t>(packet.getBody().getPayload());
                    // We compare our number with the received number
                    const auto greater_num = our_number > their_number;
                    const auto equal_num = our_number == their_number;
                    const auto greater_ip = our_ip > addr.getIp();
                    if (greater_num || (equal_num && greater_ip))
                    {
                        // We are greater than the other
                        conn.send(greater_packet, addr);
                    }
                }
                else if (packet.getType() == Packets::PacketType::SSELGT)
                {
                    // Someone else is greater than us
                    // We back off
                    someone_is_greater = true;
                }
            }
        }

        return !someone_is_greater;
    }

    void initialize()
    {
        // Estabelecer Socket
        auto conn = Sockets::UDP(Addresses::ELECTION_PORT);

        const auto our_ip = Addresses::IPv4::FromMachine();
        const auto election_finished_packet = Packets::Packet(Packets::PacketType::SSELFIN);

        auto last_seen = std::chrono::steady_clock::now();
        while (Threads::Signals::run)
        {
            if (Threads::Signals::is_manager)
            {
                // Verificar se ainda devemos ser manager
                const auto since_last_seen = std::chrono::steady_clock::now() - last_seen;
                if (since_last_seen > Threads::Delays::MANAGER_TIMEOUT)
                {
                    Threads::Signals::is_manager = false;
                    Threads::Signals::update = true;
                    Threads::Signals::update.notify_all();
                }
                last_seen = std::chrono::steady_clock::now();

                // Escutar por eleições, responder com ElectionFinished
                auto maybe_packet = conn.wait_and_receive_packet(Threads::Delays::CHECK_DELAY);
                if (!maybe_packet.has_value())
                {
                    continue;
                }
                auto &[packet, addr] = maybe_packet.value();
                if (packet.getType() == Packets::PacketType::SSEL)
                {
                    conn.send(election_finished_packet, addr);
                }
            }
            else if (!Threads::Signals::manager_found)
            {
                Threads::Signals::electing = true;

                const bool has_been_elected = elected(conn, our_ip);

                Threads::Signals::electing = false;

                if (has_been_elected)
                {
                    Threads::Signals::is_manager = true;
                    Threads::Signals::update = true;
                    Threads::Signals::update.notify_all();
                    last_seen = std::chrono::steady_clock::now();
                }
                else
                {
                    std::this_thread::sleep_for(Threads::Delays::WAIT_DELAY);
                }
            }
            else
            {
                std::this_thread::sleep_for(Threads::Delays::CHECK_DELAY);
            }
        }
        conn.close();
    }
} // namespace Subservices::Election