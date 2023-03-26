#include "Conf.hpp"
#include "savetools.hpp"
#include "zip.hpp"

namespace fs = std::filesystem;

std::mutex mutex;

void management_finished_callback(uint32_t n_removed_files) {
    uint32_t n_removed_saves = n_removed_files / 2;

    if (n_removed_saves == 0) {
        RE::DebugNotification("No save to remove");
        return;
    }

    std::string notif = fmt::format("Removed {} save", n_removed_saves);
    if (n_removed_saves > 1) {
        notif.append("s");
    }
    RE::DebugNotification(notif.c_str());
}

void manage_saves() {
    std::thread([]() {
        std::lock_guard lock(mutex);
        auto to_remove = list_saves_to_remove();
        uint32_t to_remove_size = static_cast<uint32_t>(to_remove.size());

        SPDLOG_INFO("{} files to remove", to_remove_size);

        if (to_remove_size == 0) {
            management_finished_callback(to_remove_size);
            return;
        }

        for (const auto &file : to_remove) {
            SPDLOG_INFO(file.filename().string());
        }

        if (Conf::get_singleton().compress) {
            if (!compress_saves(to_remove)) {
                SPDLOG_ERROR("Failed to compress saves, skipping removal");
                RE::DebugMessageBox("Error while compressing saves, check logs. No save has been removed.");
                return;
            }

            if (!cleanup_saves_archive()) {
                SPDLOG_ERROR("Failed to remove old saves from archive, skipping removal");
                RE::DebugMessageBox("Error while removing old saves from archive, check logs. No save has been removed.");
                return;
            }
        }

        remove_saves(to_remove);
        management_finished_callback(to_remove_size);
    }).detach();
}

bool compress_saves(const std::vector<std::filesystem::path>& files_to_compress) {
    return zip_files(files_to_compress, Conf::get_singleton().archive_path);
}

void remove_saves(const std::vector<std::filesystem::path>& files_to_remove) {
    for (const auto& file : files_to_remove) {
        fs::remove(file);
        SPDLOG_INFO("Removed {}", file.filename().string());
    }
}

bool cleanup_saves_archive() {
    // TODO cleanup archive impl
    return true;
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
    uint32_t saves_to_keep = Conf::get_singleton().n_saves_to_keep;
    if (saves_to_keep * 2 > saves.size()) {
        return std::vector<fs::path>();
    }
    // - 1 because the save that is currently being created is not in the list
    saves_to_keep--;
    // * 2 for the skse co-save
    return std::vector<fs::path>(saves.begin() + saves_to_keep * 2, saves.end());
}

uint32_t get_save_number(fs::path save_path) {
    std::string filename = save_path.filename().string();
    std::string number_str = filename.substr(4, filename.find_first_of("_"));
    return std::stoul(number_str, nullptr, 10);
}