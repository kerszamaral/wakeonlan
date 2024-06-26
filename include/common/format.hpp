#pragma once

#include <string>
#include <memory>
#include <vector>
#include <stdexcept>
#include <type_traits>
#include <ranges>
#include <algorithm>

namespace fmt
{
    template <typename E>
    constexpr auto to_underlying(const E &e) noexcept
    {
        return static_cast<std::underlying_type_t<E>>(e);
    }

    constexpr std::vector<std::string> split(const std::string &s, char seperator)
    {
        auto vec = std::vector<std::string>();

        auto rng = std::string_view(s) |
                   std::ranges::views::split(seperator) |
                   std::ranges::views::transform([](auto &&range)
                                                 { return std::string(range.begin(), range.end()); });
        for (const auto &str : rng)
        {
            vec.push_back(str);
        }

        return vec;
    }

    constexpr std::string to_lower(const std::string &s)
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

    constexpr int stoi(const std::string &str)
    {
        return stoi(str.c_str());
    }
} // namespace fmt