#ifndef SAVETOOLS_HPP
#define SAVETOOLS_HPP

#include <filesystem>

std::vector<std::filesystem::path> list_saves(bool sort = true);
std::vector<std::filesystem::path> list_saves_to_remove();

uint32_t get_save_number(std::filesystem::path save_path);

void manage_saves();
void compress_saves(const std::vector<std::filesystem::path>& files_to_compress);
void remove_saves(const std::vector<std::filesystem::path>& files_to_remove);
void cleanup_saves_archive();

#endif // SAVETOOLS_HPP