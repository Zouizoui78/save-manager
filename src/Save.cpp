#include "Save.hpp"

std::string Save::get_name() const {
    return save.stem().string();
}

uint32_t Save::get_number() const {
    std::string filename = save.filename().string();
    std::string number_str = filename.substr(4, filename.find_first_of("_"));
    return std::stoul(number_str, nullptr, 10);
}

std::filesystem::file_time_type Save::get_last_write_time() const {
    if (save.empty()) {
        return std::filesystem::file_time_type {};
    }
    return std::filesystem::last_write_time(save);
}
