#pragma once

#include <string>
#include <memory>
#include <vector>
#include <stdexcept>
#include <type_traits>

/*
g++ at the labs pcs doesn't support std::format.
This is a simple implementation of string_format using std::snprintf.
gotten from: https://stackoverflow.com/questions/2342162/stdstring-formatting-like-sprintf
*/

namespace fmt
{
    template <typename... Args>
    std::string format(const std::string &format, Args... args)
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

    template <typename E>
    constexpr auto to_underlying(const E &e) noexcept
    {
        return static_cast<std::underlying_type_t<E>>(e);
    }

    std::vector<std::string> split(const std::string &s, char seperator);

    std::string to_lower(const std::string &s);

    // https://stackoverflow.com/questions/25195176/how-do-i-convert-a-c-string-to-a-int-at-compile-time
    constexpr bool is_digit(char c)
    {
        return c <= '9' && c >= '0';
    }

    constexpr int stoi_impl(const char *str, int value = 0)
    {
        return *str ? is_digit(*str) ? stoi_impl(str + 1, (*str - '0') + value * 10)
                                     : throw "compile-time-error: not a digit"
                    : value;
    }

    constexpr int stoi(const char *str)
    {
        return stoi_impl(str);
    }
} // namespace fmt