#pragma once

#include <filesystem>
#include "Save.hpp"

namespace savetools {

std::vector<Save> list_saves();
std::vector<Save> list_saves_to_remove();

void manage_saves(const std::string &new_save_name);
bool compress_saves(const std::vector<Save>& files_to_compress);
void remove_saves(const std::vector<Save>& files_to_remove);
bool cleanup_saves_archive();

} // namespace savetools
