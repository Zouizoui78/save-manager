#include "Save.hpp"
#include "Conf.hpp"
#include "tools.hpp"

#include <cstring>

namespace fs = std::filesystem;

bool Save::is_manual(const std::string &save_name) {
    return save_name.starts_with("Save");
}

std::optional<Save>
Save::from_save_path(const std::filesystem::path &save_path) {
    std::string path_str = save_path.string();

    if (!is_manual(save_path.stem().string())) {
        return std::nullopt;
    }

    if (!path_str.ends_with(".ess")) {
        return std::nullopt;
    }

    Save ret;
    ret._save_path = save_path;
    ret._save_name = ret._save_path.stem().string();
    ret._skse_cosave_path = ret._save_path;
    ret._skse_cosave_path.replace_extension("skse");

    ret._number = parse_number(ret._save_name);
    if (ret._number == 0) {
        return std::nullopt;
    }

    ret._character_id = parse_character_id(ret._save_name);
    if (ret.get_character_id().empty()) {
        return std::nullopt;
    }

    ret._backup_path = (fs::path(Conf::get_singleton().backup_path) /
                        ret._character_id / ret._save_name)
                           .string() +
                       ".zip";

    return ret;
}

fs::path Save::get_save_path() const {
    return _save_path;
}

fs::path Save::get_skse_cosave_path() const {
    return _skse_cosave_path;
}

fs::path Save::get_backup_path() const {
    return _backup_path;
}

std::string Save::get_name() const {
    return _save_name;
}

uint32_t Save::get_number() const {
    return _number;
}

std::string Save::get_character_id() const {
    return _character_id;
}

uint32_t Save::parse_number(const std::string &save_name) {
    std::string number_match("0123456879");
    auto number_index = save_name.find_first_of(number_match);

    if (number_index == std::string::npos) {
        return 0;
    }

    auto number_end_index =
        save_name.find_first_not_of(number_match, number_index);

    if (number_end_index == std::string::npos) {
        return 0;
    }

    try {
        return std::stoul(save_name.substr(number_index, number_end_index),
                          nullptr, 10);
    }
    catch (std::invalid_argument &e) {
        SPDLOG_ERROR("Failed to get save number from '{}' : {}", save_name,
                     e.what());
    }
    catch (std::out_of_range &e) {
        SPDLOG_ERROR("Save number if out of range in '{}' : {}", save_name,
                     e.what());
    }
    return 0;
}

std::string Save::parse_character_id(const std::string &save_name) {
    auto delim_index = save_name.find("_");
    auto end_delim_index = save_name.find("_", delim_index + 1);

    if (delim_index == std::string::npos ||
        end_delim_index == std::string::npos) {
        return "";
    }

    return save_name.substr(delim_index + 1, end_delim_index - delim_index - 1);
}