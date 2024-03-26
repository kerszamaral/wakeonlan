#pragma once

/*
g++ at the labs pcs doesn't support std::format.
This is a simple implementation of string_format using std::snprintf.
gotten from: https://stackoverflow.com/questions/2342162/stdstring-formatting-like-sprintf
*/

#include <string>
#include <memory>
#include <stdexcept>

namespace fmt
{
    template <typename... Args>
    std::string string_format(const std::string &format, Args... args)
    {
        int size_s = std::snprintf(nullptr, 0, format.c_str(), args...) + 1; // Extra space for '\0'
        if (size_s <= 0)
        {
            throw std::runtime_error("Error during formatting.");
        }
        auto size = static_cast<size_t>(size_s);
        auto buf = std::make_unique<char[]>(size);
        std::snprintf(buf.get(), size, format.c_str(), args...);
        return std::string(buf.get(), buf.get() + size - 1); // We don't want the '\0' inside
    }
} // namespace fmt