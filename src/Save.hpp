#ifndef SAVE_HPP
#define SAVE_HPP

#include <filesystem>

struct Save {
    std::filesystem::path save;
    std::filesystem::path skse_cosave;

    // Returns an empty string if save and skse_cosave are empty.
    std::string get_name() const;

    // Returns default std::filesystem::file_time_type value if save and skse_cosave are empty.
    std::filesystem::file_time_type get_last_write_time() const;
};

#endif // SAVE_HPP