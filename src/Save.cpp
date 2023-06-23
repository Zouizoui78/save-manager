#include "Save.hpp"
#include "tools.hpp"

std::string Save::get_name() const {
    return save.empty() ? skse_cosave.stem().string() : save.stem().string();
}

uint32_t Save::get_number() const {
    return save.empty() ?
        tools::get_save_number(skse_cosave.filename().string()) :
        tools::get_save_number(save.filename().string());
}