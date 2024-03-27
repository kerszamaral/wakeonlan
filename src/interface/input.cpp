#include "interface/input.hpp"

#include <map>
#include <iostream>
#include <functional>
#include <string>
#include <algorithm>
#include "common/format.hpp"

typedef std::map<std::string_view, std::function<void(std::string_view)>> cmd_map_t;

cmd_map_t create_cmds(std::atomic<bool> &run)
{
    cmd_map_t cmd_map;
    cmd_map["exit"] = [&run](std::string_view args)
    {
        run.store(false);
    };

    cmd_map["wakeup"] = [](std::string_view args)
    {
        if (args.empty())
        {
            std::cout << "Usage: wakeup <hostname>" << std::endl;
            return;
        }
        std::cout << "Waking up MacAddress " << args << std::endl;
    };

    return cmd_map;
}

void run_cmd(const cmd_map_t &cmd_map, std::string_view cmd, std::string_view args)
{
    if (cmd.empty())
    {
        return;
    }

    if (cmd_map.find(cmd) != cmd_map.end())
    {
        cmd_map.at(cmd)(args);
    }
    else
    {
        std::cout << "Command not found" << std::endl;
    }
}

void ReadCin(std::atomic<bool> &run)
{
    std::string buffer;
    auto cmd_map = create_cmds(run);

    while (run.load())
    {
        std::getline(std::cin, buffer);
        std::transform(buffer.begin(), buffer.end(), buffer.begin(),
                       [](unsigned char c)
                       { return std::tolower(c); });

        auto space = buffer.find(' ');
        auto has_args = space != std::string::npos;

        std::string_view cmd = has_args ? std::string_view(buffer.c_str(), space) : buffer;
        std::string_view args = has_args ? buffer.substr(space + 1) : "";

        run_cmd(cmd_map, cmd, args);
    }
}