#include "common/atomic.hpp"

template <typename T>
auto Atomic<T>::compute(auto &&callback, auto &&...args)
{
    lock.lock();
    auto ret = callback(std::ref(resource), args...);
    lock.unlock();
    return ret;
}

template <typename T>
auto Atomic<T>::with(auto &&callback, auto &&...args)
{
    lock.lock();
    callback(std::ref(resource), args...);
    lock.unlock();
}

// return a function to be computed on a separete thread with va args+
template <typename T>
auto Atomic<T>::compute()
{
    return [this](auto &&callback, auto &&...args) -> auto
    { return this->compute(callback, args...); };
}

template <typename T>
auto Atomic<T>::with()
{
    return [this](auto &&callback, auto &&...args)
    { this->with(callback, args...); };
}