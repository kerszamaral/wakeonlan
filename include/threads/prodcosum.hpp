#pragma once

#include <queue>
#include <semaphore>
#include "common/optional.hpp"

namespace Threads
{
    template <typename T>
    class ProdCosum
    {
    private:
        std::binary_semaphore semaph = std::binary_semaphore{1};
        std::queue<T> resources;

    public:
        ProdCosum() : resources(std::queue<T>()) {}
        ProdCosum(std::queue<T> resources) : resources(resources) {}

        void produce(T resource)
        {
            semaph.acquire();
            resources.push(resource);
            semaph.release();
        }

        opt::optional<T> consume()
        {
            semaph.acquire();
            if (resources.empty())
            {
                semaph.release();
                return std::nullopt;
            }

            auto resource = resources.front();
            resources.pop();
            semaph.release();
            return resource;
        }
    };
} // namespace Threads