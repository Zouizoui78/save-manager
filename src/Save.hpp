#ifndef SAVE_HPP
#define SAVE_HPP

#include <filesystem>

struct Save {
    std::filesystem::path save;
    std::filesystem::path skse_cosave;

    std::string get_name() const;
    uint32_t get_number() const;
    std::filesystem::file_time_type get_last_write_time() const;
};

#endif // SAVE_HPP