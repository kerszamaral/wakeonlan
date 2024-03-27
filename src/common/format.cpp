#include "common/format.hpp"

#include <ranges>

std::vector<std::string> fmt::string_split(const std::string &s, char seperator)
{
    auto output = std::vector<std::string>();

    auto vec = std::string_view(s) | std::ranges::views::split(seperator);

    for (const auto &word : vec)
    {
        auto word_str = std::string_view(word);
        output.emplace_back(word_str.begin(), word_str.end());
    }

    return output;
}