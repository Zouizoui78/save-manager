#include "Save.hpp"
#include "tools.hpp"

std::string Save::get_name() const {
    return save.empty() ? skse_cosave.stem().string() : save.stem().string();
}

uint32_t Save::get_number() const {
    return save.empty() ?
        Save::get_number(skse_cosave.filename().string()) :
        Save::get_number(save.filename().string());
}

uint32_t Save::get_number(const std::string &filename) {
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

bool Save::is_manual(const std::string &save_name) {
    return save_name.starts_with("Save");
}
