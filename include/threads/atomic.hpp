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

        auto compute(auto &&callback, auto &&...args)
        {
            const std::lock_guard<std::mutex> lock_guard(lock);
            return callback(std::ref(resource), args...);
        }

        auto with(auto &&callback, auto &&...args)
        {
            const std::lock_guard<std::mutex> lock_guard(lock);
            callback(std::ref(resource), args...);
        }

        // return a function to be computed on a separete thread with va args
        auto compute()
        {
            return [this](auto &&callback, auto &&...args) -> auto
            { return this->compute(callback, args...); };
        }

        auto with()
        {
            return [this](auto &&callback, auto &&...args)
            { this->with(callback, args...); };
        }

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