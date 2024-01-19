#pragma once

#include <filesystem>
#include <vector>

namespace tools {

std::vector<std::filesystem::path> list_files_from_directory(const std::string& directory);

}
