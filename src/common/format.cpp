#include "common/format.hpp"

#include <ranges>
#include <algorithm>

std::vector<std::string> fmt::split(const std::string &s, char seperator)
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

std::string fmt::to_lower(const std::string &s)
{
    std::string output = s;
    std::transform(output.begin(), output.end(), output.begin(),
                   [](unsigned char c)
                   { return std::tolower(c); });
    return output;
}