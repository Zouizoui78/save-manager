#include "savetools.hpp"
#include "Conf.hpp"
#include "tools.hpp"
#include "ziptools.hpp"

namespace savetools {

namespace fs = std::filesystem;

const Conf &conf = Conf::get_singleton();
std::mutex mutex;

std::vector<Save> list_character_saves(const std::string &character_id);
std::vector<Save> list_saves_to_remove(const std::string &character_id);
bool compress_saves(const std::vector<Save> &files_to_compress);
void remove_saves(const std::vector<Save> &files_to_remove);
bool cleanup_saves_archive(const std::string &character_id);

void manage_saves(const std::string &new_save_name) {
    if (!Save::is_manual(new_save_name)) {
        SPDLOG_DEBUG("Not a manual save, skipping");
        return;
    }

    std::thread([&new_save_name]() {
        std::lock_guard lock(mutex);

        std::string character_id = Save::parse_character_id(new_save_name);
        if (character_id.empty()) {
            return;
        }

        auto to_remove = list_saves_to_remove(character_id);
        uint32_t to_remove_size = static_cast<uint32_t>(to_remove.size());

        SPDLOG_INFO("{} saves to remove", to_remove_size);

        if (to_remove_size == 0) {
            return;
        }

        if (conf.backup) {
            if (!compress_saves(to_remove)) {
                SPDLOG_ERROR("Failed to compress saves, skipping removal");
                RE::DebugMessageBox("Error while compressing saves, check "
                                    "logs. No save has been removed.");
                return;
            }

            if (conf.max_backed_up_saves > 0 &&
                !cleanup_saves_archive(character_id)) {
                SPDLOG_ERROR(
                    "Failed to remove one or more old compressed saves");
                RE::DebugMessageBox(
                    "Error while removing old saves from archive, check logs. "
                    "No save has been removed.");
                return;
            }
        }

        remove_saves(to_remove);

        std::string notif = std::format("Removed {} save", to_remove_size);
        if (to_remove_size > 1) {
            notif.append("s");
        }
        RE::DebugNotification(notif.c_str());
    }).detach();
}

bool compress_saves(const std::vector<Save> &saves) {
    bool ok = true;

    fs::create_directories(saves[0].get_backup_path().parent_path().string());

    for (const auto &save : saves) {
        std::string archive_path(save.get_backup_path().string());
        SPDLOG_DEBUG("Compressing save '{}' to '{}'", save.get_name(),
                     archive_path);

        std::vector<fs::path> files{save.get_save_path()};
        auto skse_cosave_path = save.get_skse_cosave_path();

        if (fs::exists(skse_cosave_path)) {
            files.emplace_back(skse_cosave_path);
        }

        if (!ziptools::zip_files(files, archive_path)) {
            ok = false;
        }
    }
    return ok;
}

void remove_saves(const std::vector<Save> &saves) {
    for (const auto &save : saves) {
        SPDLOG_INFO("Removing save '{}'", save.get_name());
        fs::remove(save.get_save_path());

        if (fs::exists(save.get_skse_cosave_path())) {
            fs::remove(save.get_skse_cosave_path());
        }
    }
}

bool cleanup_saves_archive(const std::string &character_id) {
    auto compressed_saves_files = tools::list_files_from_directory(
        (fs::path(conf.backup_path) / character_id).string());

    std::vector<Save> compressed_saves;
    for (const auto &file : compressed_saves_files) {
        auto save = Save::from_save_path(file.string());
        if (save.has_value()) {
            compressed_saves.emplace_back(std::move(*save));
        }
    }

    if (compressed_saves.size() <= conf.max_backed_up_saves) {
        return true;
    }

    std::ranges::sort(compressed_saves, [](const Save &a, const Save &b) {
        return a.get_number() > b.get_number();
    });

    bool ok = true;
    std::for_each(compressed_saves.begin() + conf.max_backed_up_saves,
                  compressed_saves.end(), [&ok](const Save &save) {
                      SPDLOG_INFO("Removing file '{}'",
                                  save.get_save_path().string());
                      if (!fs::remove(save.get_save_path())) {
                          SPDLOG_ERROR("Failed to remove file '{}'",
                                       save.get_save_path().string());
                          ok = false;
                      }
                  });
    return ok;
}

std::vector<Save> list_character_saves(const std::string &character_id) {
    auto files = tools::list_files_from_directory(conf.saves_path);
    std::vector<Save> saves;

    for (const auto &file : files) {
        auto save = Save::from_save_path(file.string());

        if (!save.has_value()) {
            continue;
        }

        if (save->get_character_id() != character_id) {
            continue;
        }

        saves.emplace_back(std::move(*save));
    }

    return saves;
}

std::vector<Save> list_saves_to_remove(const std::string &character_id) {
    auto saves = list_character_saves(character_id);

    std::ranges::sort(saves, [](const Save &left, const Save &right) {
        return left.get_number() > right.get_number();
    });

    uint32_t saves_to_keep = conf.n_saves_to_keep;
    if (saves_to_keep > saves.size()) {
        return std::vector<Save>();
    }
    // - 1 because the save that is currently being created is not in the list
    return std::vector<Save>(saves.begin() + saves_to_keep - 1, saves.end());
}

} // namespace savetools