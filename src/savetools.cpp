#include "Conf.hpp"
#include "savetools.hpp"
#include "zip.hpp"

namespace fs = std::filesystem;

void manage_saves() {
    auto to_remove = list_saves_to_remove();

    SPDLOG_INFO("Found {} files to remove", to_remove.size());
    for (const auto &file : to_remove) {
        SPDLOG_INFO(file.filename().string());
    }

    if (!Conf::get_singleton().compress) {
        remove_saves(to_remove);
        return;
    }

    std::thread t([to_remove]() {
        compress_saves(to_remove);
        cleanup_saves_archive();
        remove_saves(to_remove);
    });
    t.detach();
}

void compress_saves(const std::vector<std::filesystem::path>& files_to_compress) {
    zip_files(files_to_compress, Conf::get_singleton().archive_path);
}

void remove_saves(const std::vector<std::filesystem::path>& files_to_remove) {
    for (const auto& file : files_to_remove) {
        fs::remove(file);
    }
}

void cleanup_saves_archive() {
    // TODO cleanup archive impl
}

std::vector<fs::path> list_files_from_directory(const std::string &directory) {
    std::vector<fs::path> ret;
    for (const auto &dir_entry : fs::directory_iterator(directory)) {
        ret.emplace_back(dir_entry.path());
    }
    return ret;
}

std::vector<fs::path> list_saves(bool sort) {
    auto files = list_files_from_directory(Conf::get_singleton().saves_path);
    for (auto it = files.begin() ; it != files.end() ; ) {
        std::string ext = it->extension().string();
        if (ext != ".ess" && ext != ".skse") {
            it = files.erase(it);
        }
        else {
            ++it;
        }
    }

    if (sort) {
        std::sort(files.begin(), files.end(), [](fs::path left, fs::path right) {
            return fs::last_write_time(left) > fs::last_write_time(right);
        });
    }

    return files;
}

std::vector<fs::path> list_saves_to_remove() {
    auto saves = list_saves();
    // - 1 because the save that is currently being created is not in the list
    uint16_t saves_to_keep = Conf::get_singleton().n_saves_to_keep - 1;
    // * 2 for the skse co-save
    return std::vector<fs::path>(saves.begin() + saves_to_keep * 2, saves.end());
}

uint64_t get_save_number(fs::path save_path) {
    std::string filename = save_path.filename().string();
    std::string number_str = filename.substr(4, filename.find_first_of("_"));
    return std::stoul(number_str, nullptr, 10);
}