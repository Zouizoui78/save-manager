#ifndef TOOLS_HPP
#define TOOLS_HPP

#include <filesystem>
#include <vector>

namespace tools {

std::vector<std::filesystem::path> list_files_from_directory(const std::string& directory);

uint32_t get_save_number(const std::string &filename);

bool is_manual_save(const std::string &save_name);

}

#endif // TOOLS_HPP