#ifndef SAVE_HPP
#define SAVE_HPP

#include <filesystem>

struct Save {
    std::filesystem::path save;
    std::filesystem::path skse_cosave;

    // Returns an empty string if save and skse_cosave are empty.
    std::string get_name() const;

    uint32_t get_number() const;
};

#endif // SAVE_HPP