#ifndef TOOLS_HPP
#define TOOLS_HPP

#include <filesystem>
#include <vector>

namespace tools {

std::vector<std::filesystem::path> list_files_from_directory(const std::string& directory);

}

#endif // TOOLS_HPP