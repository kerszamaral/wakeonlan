#include "subservices/interface/output.hpp"

#include <thread>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <cstring>
#include "common/platform.hpp"
#include "common/pcinfo.hpp"
#include "threads/signals.hpp"
#include <syncstream>

namespace Subservices::Interface::Output
{
    // https://github.com/paulkazusek/std_format_cheatsheet?tab=readme-ov-file#what-is-stdformat
    std::string make_entry(const std::string &str, size_t size)
    {
        std::stringstream ss;
        ss << std::left << std::setw(size) << str;
        return ss.str();
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
                tout << "\033[2J\033[1;1H";
#endif
                tout << table << std::endl;
            }

            Threads::Signals::update.wait(false);
        }
    }
}