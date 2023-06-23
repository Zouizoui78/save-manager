#include "tools.hpp"

namespace fs = std::filesystem;

namespace tools {

std::vector<fs::path> list_files_from_directory(const std::string& directory) {
    std::vector<fs::path> ret;
    for (const auto& dir_entry : fs::directory_iterator(directory)) {
        ret.emplace_back(dir_entry.path());
    }
    return ret;
}

uint32_t get_save_number(const std::string &filename) {
    std::string number_match("0123456879");
    auto number_index = filename.find_first_of(number_match);
    auto number_end_index = filename.find_first_not_of(number_match, number_index);
    try {
        return std::stoul(filename.substr(number_index, number_end_index), nullptr, 10);
    }
    catch (std::invalid_argument &e) {
        SPDLOG_ERROR("Failed to get save number from '{}' : {}", filename, e.what());
    }
    catch (std::out_of_range &e) {
        SPDLOG_ERROR("Save number if out of range in '{}' : {}", filename, e.what());
    }
    return 0;
}

bool is_manual_save(const std::string &save_name) {
    return save_name.starts_with("Save");
}

}