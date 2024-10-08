#pragma once

#include <queue>
#include <mutex>
#include "common/optional.hpp"

namespace Threads
{
    template <typename T>
    class AtomicQueue
    {
    private:
        std::mutex lock = std::mutex();
        std::queue<T> resources;

    public:
        AtomicQueue() : resources(std::queue<T>()) {}
        AtomicQueue(std::queue<T> resources) : resources(resources) {}

        void produce(T resource)
        {
            const std::lock_guard<std::mutex> lock_guard(lock);
            resources.push(resource);
        }

        opt::optional<T> consume()
        {
            const std::lock_guard<std::mutex> lock_guard(lock);
            if (resources.empty())
            {
                return opt::nullopt;
            }

            auto resource = resources.front();
            resources.pop();
            return resource;
        }
    };
} // namespace Threads