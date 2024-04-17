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
        return ss.str();
    }

#ifdef OS_WIN
    constexpr auto CLEAR = "cls";
#else
    constexpr auto CLEAR = "clear";
#endif

    void WriteCout(PC::atomic_pc_map_t &pc_map)
    {
        while (Threads::Signals::run)
        {
            const std::string &table = pc_map.compute(make_pc_table);
#ifndef DEBUG
            std::system(CLEAR);
#endif
            std::cout << table << std::endl;

            Threads::Signals::update = false;
            Threads::Signals::update.wait(false);
        }
    }
}