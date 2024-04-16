#include "subservices/interface/input.hpp"

#include <map>
#include <iostream>
#include <functional>
#include <string>
#include <algorithm>
#include "common/platform.hpp"
#include "common/format.hpp"
#include "common/optional.hpp"
#include "threads/signals.hpp"

namespace Subservices::Interface::Input
{
    typedef std::map<std::string_view, std::function<void(std::string_view)>> cmd_map_t;

    cmd_map_t create_cmds(PC::wakeups_queue &wakeups)
    {
        cmd_map_t cmd_map;
        cmd_map["exit"] = [](std::string_view args)
        {
            Threads::Signals::run = false;
        };

        cmd_map["wakeup"] = [&wakeups](std::string_view args)
        {
            if (args.empty())
            {
                std::cout << "Usage: wakeup <hostname>" << std::endl;
                return;
            }
            std::string hostname(args.begin(), args.end());
            wakeups.produce(hostname);
        };

        return cmd_map;
    }

    void run_cmd(const cmd_map_t &cmd_map, std::string_view cmd, std::string_view args)
    {
        if (cmd.empty())
        {
            return;
        }

        if (cmd_map.contains(cmd))
        {
            cmd_map.at(cmd)(args);
        }
        else
        {
            std::cout << "Command not found" << std::endl;
        }
    }

    // https://stackoverflow.com/questions/65359021/how-to-define-a-non-blocking-input-in-c
    bool stdinHasData()
    {
#ifdef OS_WIN
        // this works by harnessing Windows' black magic:
        return _kbhit();
#else
        // using a timeout of 0 so we aren't waiting:
        struct timespec timeout
        {
            0l, 0l
        };

        // create a file descriptor set
        fd_set fds{};

        // initialize the fd_set to 0
        FD_ZERO(&fds);
        // set the fd_set to target file descriptor 0 (STDIN)
        FD_SET(0, &fds);

        // pselect the number of file descriptors that are ready, since
        //  we're only passing in 1 file descriptor, it will return either
        //  a 0 if STDIN isn't ready, or a 1 if it is.
        return pselect(0 + 1, &fds, nullptr, nullptr, &timeout, nullptr) == 1;
#endif
    }

    opt::optional<std::string> async_getline()
    {
        if (!stdinHasData())
        {
            return opt::nullopt;
        }

        std::string buffer;
        std::getline(std::cin, buffer);
        if (buffer.empty())
        {
            return opt::nullopt;
        }
        return buffer;
    }

    void ReadCin(PC::wakeups_queue &wakeups)
    {
        auto cmd_map = create_cmds(wakeups);

        while (Threads::Signals::run)
        {
            auto buffer = async_getline();

            if (!buffer.has_value())
            {
                continue;
            }

            auto cmd_args = fmt::split(fmt::to_lower(buffer.value()), ' ');
            std::string_view cmd = cmd_args[0];
            std::string_view args = cmd_args.size() > 1 ? cmd_args[1] : "";

            run_cmd(cmd_map, cmd, args);
        }
    }
}