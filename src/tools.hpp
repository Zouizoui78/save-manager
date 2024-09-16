#pragma once

#include <filesystem>
#include <vector>

namespace tools {

std::vector<std::filesystem::path>
list_files(const std::filesystem::path &directory);

std::filesystem::path get_documents_path();

bool zip_files(const std::vector<std::filesystem::path> &files,
               const std::string &dest);

} // namespace tools
