#ifndef SAVETOOLS_HPP
#define SAVETOOLS_HPP

#include <filesystem>
#include "Save.hpp"

std::vector<Save> list_saves();
std::vector<Save> list_saves_to_remove();

void manage_saves();
bool compress_saves(const std::vector<Save>& files_to_compress);
void remove_saves(const std::vector<Save>& files_to_remove);
bool cleanup_saves_archive();

#endif // SAVETOOLS_HPP