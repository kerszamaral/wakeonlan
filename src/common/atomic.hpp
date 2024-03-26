#pragma once

#include <iostream>
#include <functional>
#include <mutex>

template <typename T>
class Atomic
{
private:
    std::mutex lock;
    T resource;

public:
    Atomic() : resource(T()) {}
    Atomic(T value) : resource(value) {}

    auto compute(auto &&callback, auto &&...args);

    auto with(auto &&callback, auto &&...args);

    // return a function to be computed on a separete thread with va args
    auto compute();

    auto with();

    friend std::ostream &operator<<(std::ostream &os, const Atomic &atomic)
    {
        os << atomic.resource;
        return os;
    }
};