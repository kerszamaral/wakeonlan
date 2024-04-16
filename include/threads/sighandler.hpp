#pragma once

namespace Threads::SigHandler
{
    void setup();

    void teardown();

    void run_handler(int signal);
}
