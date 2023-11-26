#pragma once

#include <string>
#include <unordered_map>
#include <vector>

std::string join_strings(const std::vector<std::string>& strs, size_t offset, char ch);
std::unordered_map<std::string, std::string> get_commands_from_text(const std::string& text);