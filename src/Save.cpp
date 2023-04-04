#include "Save.hpp"

std::string Save::get_name() const {
    return save.empty() ? skse_cosave.stem().string() : save.stem().string();
}

std::filesystem::file_time_type Save::get_last_write_time() const {
    std::filesystem::path path = save.empty() ? skse_cosave : save;
    if (path.empty()) {
        return std::filesystem::file_time_type {};
    }
    return std::filesystem::last_write_time(path);
}
