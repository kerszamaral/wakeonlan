// #include "common/signal.hpp"

// #include <iostream>

// void signal_handler(int signum, auto &&callback)
// {
//     std::cout << "Signal " << signum << " received." << std::endl;
//     callback();
//     exit(signum);
// }

// void set_signal_handler(int signum, auto &&callback)
// {
//     struct sigaction sigIntHandler;
//     sigIntHandler.sa_handler = signal_handler;
//     sigemptyset(&sigIntHandler.sa_mask);
//     sigIntHandler.sa_flags = 0;

//     sigaction(signum, &sigIntHandler, NULL);
// }