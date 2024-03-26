#pragma once

#include <functional>
#include <csignal>

std::function<void(int)> shutdown_handler;
void signal_handler(int signal)
{
    shutdown_handler(signal);
}