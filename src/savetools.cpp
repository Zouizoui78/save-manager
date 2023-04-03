#include "Conf.hpp"
#include "savetools.hpp"
#include "zip.hpp"

namespace fs = std::filesystem;

std::mutex mutex;

void management_finished_callback(uint32_t n_removed_saves) {
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

        SPDLOG_INFO("{} saves to remove", to_remove_size);

        if (to_remove_size == 0) {
            management_finished_callback(to_remove_size);
            return;
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

bool compress_saves(const std::vector<Save>& saves) {
    bool error = false;
    const Conf& conf = Conf::get_singleton();
    for (const auto& save : saves) {
        fs::path archive_path { fs::path(conf.backup_path) / (save.get_name() + ".zip") };
        if (!zip_files({ save.save, save.skse_cosave }, archive_path.string())) {
            error = true;
        }
    }
    return !error;
}

void remove_saves(const std::vector<Save>& saves) {
    for (const auto& save : saves) {
        fs::remove(save.save);
        fs::remove(save.skse_cosave);
    }
}

bool cleanup_saves_archive() {
    // TODO cleanup archive impl
    return true;
}

std::vector<fs::path> list_files_from_directory(const std::string& directory) {
    std::vector<fs::path> ret;
    for (const auto& dir_entry : fs::directory_iterator(directory)) {
        ret.emplace_back(dir_entry.path());
    }
    return ret;
}

std::vector<Save> list_saves() {
    auto files = list_files_from_directory(Conf::get_singleton().saves_path);
    std::vector<Save> saves;
    for (const auto& file : files) {
        std::string ext = file.extension().string();
        if (ext != ".ess" && ext != ".skse") {
            continue;
        }

        auto save_it = std::find_if(saves.begin(), saves.end(), [&](const Save& save) {
            return save.get_name() == file.stem().string();
        });

        if (save_it == saves.end()) {
            saves.emplace_back();
            save_it = saves.end() - 1;
        }

        if (ext == ".ess") {
            save_it->save = file;
        }
        else if (ext == ".skse") {
            save_it->skse_cosave = file;
        }
    }

    return saves;
}

std::vector<Save> list_saves_to_remove() {
    auto saves = list_saves();

    std::sort(saves.begin(), saves.end(), [](const Save& left, const Save& right) {
        return left.get_last_write_time() > right.get_last_write_time();
    });

    uint32_t saves_to_keep = Conf::get_singleton().n_saves_to_keep;
    if (saves_to_keep > saves.size()) {
        return std::vector<Save>();
    }
    // - 1 because the save that is currently being created is not in the list
    saves_to_keep--;
    return std::vector<Save>(saves.begin() + saves_to_keep, saves.end());
}