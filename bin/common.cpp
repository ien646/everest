#include "common.h"

#include <ien/str_utils.hpp>

std::string join_strings(const std::vector<std::string>& strs, size_t offset, char ch)
{
    std::string result;
    if(strs.size() == 0)
    {
        return {};
    }
    if(strs.size() == 1)
    {
        if(offset > 0)
        {
            return {};
        }
        return strs[0];
    }

    for(size_t i = offset; i < strs.size(); ++i)
    {
        result += strs[i];
        result += ch;
    }
    result += strs.back();

    return result;
}

std::unordered_map<std::string, std::string> get_commands_from_text(const std::string& text)
{
    std::unordered_map<std::string, std::string> commands;
    for(const auto& line : ien::str_split(text, '\n'))
    {
        auto segments = ien::str_split(std::string{line}, ':');
        if(segments.size() > 2)
        {
            segments = {segments[0], join_strings(segments, 1, ':')};
        }
        commands.emplace(segments[0], segments[1]);
    }
    return commands;
}