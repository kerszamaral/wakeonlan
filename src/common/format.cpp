#include "common/format.hpp"

#include <ranges>
#include <algorithm>
#include <sstream>

std::vector<std::string> fmt::split(const std::string &s, char seperator)
{
    auto output = std::vector<std::string>();

    auto vec = std::string_view(s) | std::ranges::views::split(seperator);

    std::stringstream ss;
    for (const auto &word : vec)
    {
        for (const auto &c : word)
        {
            ss << c;
        }
        output.push_back(ss.str());
        ss.str(std::string());
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