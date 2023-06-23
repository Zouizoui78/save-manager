#include "Conf.hpp"
#include "savetools.hpp"
#include "tools.hpp"
#include "zip.hpp"

namespace fs = std::filesystem;

const Conf &conf = Conf::get_singleton();
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

void manage_saves(const std::string &new_save_name) {
    if (!tools::is_manual_save(new_save_name)) {
        SPDLOG_DEBUG("Not a manual save, skipping");
        return;
    }

    std::thread([]() {
        std::lock_guard lock(mutex);
        auto to_remove = list_saves_to_remove();
        uint32_t to_remove_size = static_cast<uint32_t>(to_remove.size());

        SPDLOG_INFO("{} saves to remove", to_remove_size);

        if (to_remove_size == 0) {
            management_finished_callback(to_remove_size);
            return;
        }

        if (conf.compress) {
            if (!compress_saves(to_remove)) {
                SPDLOG_ERROR("Failed to compress saves, skipping removal");
                RE::DebugMessageBox("Error while compressing saves, check logs. No save has been removed.");
                return;
            }

            if (conf.max_backed_up_saves > 0 && !cleanup_saves_archive()) {
                SPDLOG_ERROR("Failed to remove one or more old compressed saves");
                RE::DebugMessageBox("Error while removing old saves from archive, check logs. No save has been removed.");
                return;
            }
        }

        remove_saves(to_remove);
        management_finished_callback(to_remove_size);
    }).detach();
}

bool compress_saves(const std::vector<Save>& saves) {
    bool ok = true;
    for (const auto& save : saves) {
        fs::path archive_path { fs::path(conf.backup_path) / (save.get_name() + ".zip") };
        SPDLOG_DEBUG("Compressing save '{}' to '{}'", save.get_name(), archive_path.string());
        if (!zip_files({ save.save, save.skse_cosave }, archive_path.string())) {
            ok = false;
        }
    }
    return ok;
}

void remove_saves(const std::vector<Save>& saves) {
    for (const auto& save : saves) {
        SPDLOG_INFO("Removing save '{}'", save.get_name());
        fs::remove(save.save);
        fs::remove(save.skse_cosave);
    }
}

bool cleanup_saves_archive() {
    auto compressed_saves = tools::list_files_from_directory(conf.backup_path);

    // nothing to do
    if (compressed_saves.size() <= conf.max_backed_up_saves) {
        return true;
    }

    std::ranges::sort(compressed_saves, [](const fs::path &a, const fs::path &b) {
        return tools::get_save_number(a.filename().string()) > tools::get_save_number(b.filename().string());
    });

    bool ok = true;
    std::for_each(compressed_saves.begin() + conf.max_backed_up_saves, compressed_saves.end(), [&ok](const fs::path &file) {
        SPDLOG_INFO("Removing file '{}'", file.string());
        if (!fs::remove(file)) {
            SPDLOG_ERROR("Failed to remove file '{}'", file.string());
            ok = false;
        }
    });
    return ok;
}

std::vector<Save> list_saves() {
    auto files = tools::list_files_from_directory(conf.saves_path);
    std::vector<Save> saves;
    for (const auto& file : files) {
        std::string ext = file.extension().string();
        if (ext != ".ess" && ext != ".skse") {
            continue;
        }

        if (!tools::is_manual_save(file.filename().string())) {
            SPDLOG_DEBUG("Skipping file '{}'", file.filename().string());
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
        return left.get_number() > right.get_number();
    });

    uint32_t saves_to_keep = conf.n_saves_to_keep;
    if (saves_to_keep > saves.size()) {
        return std::vector<Save>();
    }
    // - 1 because the save that is currently being created is not in the list
    return std::vector<Save>(saves.begin() + saves_to_keep - 1, saves.end());
}