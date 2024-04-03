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
            lock.lock();
            auto ret = callback(std::ref(resource), args...);
            lock.unlock();
            return ret;
        }

        auto with(auto &&callback, auto &&...args)
        {
            lock.lock();
            callback(std::ref(resource), args...);
            lock.unlock();
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

        friend std::ostream &operator<<(std::ostream &os, const Atomic &atomic)
        {
            os << atomic.resource;
            return os;
        }
    };
} // namespace Threads