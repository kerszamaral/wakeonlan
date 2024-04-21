#include "subservices/interface/output.hpp"

#include <thread>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <cstring>
#include <format>
#include "common/platform.hpp"
#include "common/pcinfo.hpp"
#include "threads/signals.hpp"
#include <syncstream>

namespace Subservices::Interface::Output
{
    // https://github.com/paulkazusek/std_format_cheatsheet?tab=readme-ov-file#what-is-stdformat
    std::string make_entry(const std::string &str, size_t size)
    {
        const std::string_view fmt = "{:<" + std::to_string(size) + "}";
        return std::vformat(fmt, std::make_format_args(str));
    }

    std::string make_pc_table(const PC::pc_map_t &pc_map)
    {
        std::stringstream ss;
        constexpr auto MANAGER_TAG = " *";
        const std::vector<std::pair<std::string, size_t>> headers = {
            {std::string("Hostname") + (Threads::Signals::is_manager ? MANAGER_TAG : ""), 20},
            {"MAC Address", 20},
            {"IPv4 Address", 17},
            {"Status", 8},
        };
        for (const auto &[header, size] : headers)
        {
            ss << make_entry(header, size);
        }
        ss << "\n";

        for (auto &pc : pc_map)
        {
            const auto hostname = pc.second.get_hostname() + (pc.second.get_is_manager() ? MANAGER_TAG : "");
            ss << make_entry(hostname, headers[0].second);

            const auto &mac = pc.second.get_mac().to_string();
            ss << make_entry(mac, headers[1].second);

            const auto &ipv4 = pc.second.get_ipv4().to_string();
            ss << make_entry(ipv4, headers[2].second);

            const auto &status = pc.second.get_status() == PC::STATUS::AWAKE ? "Awake" : "Asleep";
            ss << make_entry(status, headers[3].second) << "\n";
        }
        Threads::Signals::update = false;
        return ss.str();
    }

    void WriteCout(PC::atomic_pc_map_t &pc_map)
    {
        while (Threads::Signals::run)
        {
            const std::string &table = pc_map.execute(make_pc_table);
            {
                std::osyncstream tout(std::cout);
#ifndef DEBUG
                //? https://stackoverflow.com/questions/6486289/how-can-i-clear-console
#ifdef DISABLED_OS_WIN
                COORD topLeft = {0, 0};
                HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
                CONSOLE_SCREEN_BUFFER_INFO screen;
                DWORD written;

                GetConsoleScreenBufferInfo(console, &screen);
                FillConsoleOutputCharacterA(
                    console, ' ', screen.dwSize.X * screen.dwSize.Y, topLeft, &written);
                FillConsoleOutputAttribute(
                    console, FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_BLUE,
                    screen.dwSize.X * screen.dwSize.Y, topLeft, &written);
                SetConsoleCursorPosition(console, topLeft);
#else
                tout << "\x1B[2J\x1B[H";
#endif
#endif
                tout << table << std::endl;
            }

            Threads::Signals::update.wait(false);
        }
    }
}