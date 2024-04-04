#pragma once

#include <queue>
#include <optional>
#include <semaphore>

namespace Threads
{
    template <typename T>
    class ProdCosum
    {
    private:
        std::binary_semaphore semaph = std::binary_semaphore{1};
        std::queue<T> resources;

    public:
        ProdCosum() : resources(std::vector<T>()) {}
        ProdCosum(std::queue<T> resources) : resources(resources) {}

        void produce(T resource)
        {
            std::lock_guard<std::binary_semaphore> lock(semaph);
            resources.push(resource);
        }

        std::optional<T> consume()
        {
            std::lock_guard<std::binary_semaphore> lock(semaph);
            if (resources.empty())
            {
                return std::nullopt;
            }

            auto resource = resources.front();
            resources.pop();
            return resource;
        }
    };
} // namespace Threads