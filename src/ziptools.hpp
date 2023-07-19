#ifndef ZIPTOOLS_HPP
#define ZIPTOOLS_HPP

#include <filesystem>

namespace ziptools {

bool zip_files(const std::vector<std::filesystem::path>& files, const std::string& dest);

} // namespace ziptools

#endif // ZIPTOOLS_HPP