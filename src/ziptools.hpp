#pragma once

#include <filesystem>

namespace ziptools {

bool zip_files(const std::vector<std::filesystem::path>& files, const std::string& dest);

} // namespace ziptools
