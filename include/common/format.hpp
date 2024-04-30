#pragma once

#include <string>
#include <memory>
#include <vector>
#include <stdexcept>
#include <type_traits>
#include <ranges>
#include <algorithm>
#include <sstream>

namespace fmt
{

    /*
    g++ at the labs pcs doesn't support std::format.
    This is a simple implementation of string_format using std::snprintf.
    gotten from: https://stackoverflow.com/questions/2342162/stdstring-formatting-like-sprintf
    */
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

    inline std::vector<std::string> split(const std::string &s, char seperator)
    {
        auto output = std::vector<std::string>();

        auto vec = std::string_view(s) | std::ranges::views::split(seperator);

        std::string ss;
        for (const auto &word : vec)
        {
            for (const auto &c : word)
            {
                ss += c;
            }
            output.emplace_back(ss);
            ss.clear();
        }

        return output;
    }

    inline std::string to_lower(const std::string &s)
    {
        std::string output = s;
        std::transform(output.begin(), output.end(), output.begin(),
                       [](unsigned char c)
                       { return std::tolower(c); });
        return output;
    }

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

    inline int stoi(const std::string &str)
    {
        return stoi(str.c_str());
    }
} // namespace fmt