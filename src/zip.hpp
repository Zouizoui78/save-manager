#ifndef ZIP_HPP
#define ZIP_HPP

#include <filesystem>

void zip_files(const std::vector<std::filesystem::path>& files, const std::string& dest);

#endif // ZIP_HPP