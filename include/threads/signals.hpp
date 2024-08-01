#pragma once

#include <atomic>

namespace Threads
{
    class Signals
    {
        /*
        ? We can use the atomic variables just like any other, they will be atomic-ed by the compiler.
            https://en.cppreference.com/w/cpp/atomic/atomic
            https://en.cppreference.com/w/cpp/atomic/atomic/operator_T
            https://en.cppreference.com/w/cpp/atomic/atomic/operator%3D
        */
    public:
        static std::atomic_bool is_manager;
        static std::atomic_bool run;
        static std::atomic_bool update;
        static std::atomic_bool manager_found;
        static std::atomic_bool replication_update;
        static std::atomic_bool electing;
        static std::atomic_uint32_t table_version;

        Signals() = delete;
        ~Signals() = delete;
    };
} // namespace Threads