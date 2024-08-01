#include "subservices/interface/input.hpp"

#include <map>
#include <iostream>
#include <functional>
#include <string>
#include <algorithm>
#include "common/optional.hpp"
#include <syncstream>
#include <thread>

#include "common/platform.hpp"
#include "common/format.hpp"
#include "threads/signals.hpp"
#include "threads/delays.hpp"
#include "threads/sighandler.hpp"

namespace Subservices::Interface::Input
{
    typedef std::map<std::string_view, std::function<void(std::string_view)>> cmd_map_t;

    cmd_map_t create_cmds(PC::wakeups_queue &wakeups)
    {
        cmd_map_t cmd_map;
        cmd_map["exit"] = [](std::string_view args __attribute__((unused)))
        {
            Threads::SigHandler::run_handler(0);
        };

        cmd_map["wakeup"] = [&wakeups](std::string_view args)
        {
            //? The specification says that only the manager can send wakeups
            if (!Threads::Signals::is_manager)
            {
                std::osyncstream(std::cout) << "Only the manager can send wakeups" << std::endl;
                return;
            }

            if (args.empty())
            {
                std::osyncstream(std::cout) << "Usage: wakeup <hostname>" << std::endl;
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
            std::osyncstream(std::cout) << "Command not found" << std::endl;
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
            /*
            ? The buffer can be empty for two reasons:
            ? the user either pressed enter without typing anything
            ? or the user pressed Ctrl+D (EOF) [On Windows, it's Ctrl+Z, but you need to press Enter after that]
            ? We differentiate between the two by checking std::feof(stdin)
            */
            if (std::feof(stdin))
            {
                //! The specification says that the user can exit by pressing Ctrl+D
                Threads::Signals::run = false;
                Threads::Signals::run.notify_all();
            }
            return opt::nullopt;
        }
        return buffer;
    }

    void ReadCin(PC::wakeups_queue &wakeups)
    {
        auto cmd_map = create_cmds(wakeups);

        while (Threads::Signals::run)
        {
            std::this_thread::sleep_for(Threads::Delays::INPUT_DELAY);
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