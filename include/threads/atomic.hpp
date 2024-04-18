#pragma once

#include <iostream>
#include <functional>
#include <mutex>

namespace Threads
{
    template <typename T>
    class Atomic
    {
    private:
        std::mutex lock = std::mutex();
        T resource;

    public:
        Atomic() : resource(T()) {}
        Atomic(T value) : resource(value) {}

        template <class F>
        auto execute(F &&f, auto &&...args)
        {
            const std::lock_guard<std::mutex> lock_guard(lock);
            return std::invoke(std::forward<F>(f), std::ref(resource), args...);
        }

        friend std::ostream &operator<<(std::ostream &os, const Atomic &atomic)
        {
            os << atomic.resource;
            return os;
        }
    };
} // namespace Threads