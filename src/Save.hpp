#pragma once

#include <filesystem>
#include <optional>

// Class that parses a save filename and exposes some information about it.
// Save name are formatted like so :
// <slot><number>_<character_id>_<modded>_<character_name_in_hex>_<cell>_<playtime>_<timestamp>_<level>_1.ess
// Where
//  - slot : either Quicksave0, Autosave{1,2,3} or Save{1,2,3,...}
//  - number : the save number
//  - character_id : unique ID generated each time the Racemenu is closed
//  - modded : flag indicating whether the game is modded or not
//  - character_name_in_hex : character name with each character printed in
//  hexadecimal
//  - cell : ingame cell where the save was created

class Save {
public:
    // Return true if save is a manual save, false if it's a quicksave, an
    // automatic save or another file.
    static bool is_manual(const std::string &save_name);
    static uint32_t parse_number(const std::string &save_name);
    static std::string parse_character_id(const std::string &save_name);

    // Return nullopt if parsing fails.
    static std::optional<Save>
    from_save_path(const std::filesystem::path &save_path);

    std::filesystem::path get_save_path() const;
    std::filesystem::path get_skse_cosave_path() const;
    std::filesystem::path get_backup_path() const;
    std::string get_name() const;
    uint32_t get_number() const;
    std::string get_character_id() const;

private:
    std::filesystem::path _save_path;
    std::filesystem::path _skse_cosave_path;
    std::filesystem::path _backup_path;
    std::string _save_name;

    uint32_t _number;
    std::string _character_id;
};
