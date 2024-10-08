#include "savetools.hpp"
#include "Conf.hpp"
#include "tools.hpp"

#include "tools/utils/stopwatch.hpp"
#include "tools/utils/thread_pool.hpp"

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

    std::thread([new_save_name]() {
        tools::utils::Stopwatch sw;
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

        auto duration_s = sw.get_duration<std::chrono::seconds>();
        std::string notif =
            std::format("Removed {} save{} in {:.3f}s", to_remove_size,
                        to_remove_size > 1 ? "s" : "", duration_s);
        RE::DebugNotification(notif.c_str());
    }).detach();
}

bool compress_saves(const std::vector<Save> &saves) {
    std::atomic<bool> ok = true;

    fs::create_directories(saves[0].get_backup_path().parent_path().string());

    auto n_threads = std::thread::hardware_concurrency() / 2;
    tools::ThreadPool pool(n_threads < 1 ? 1 : n_threads);

    for (const auto &save : saves) {
        std::string archive_path(save.get_backup_path().string());
        SPDLOG_DEBUG("Compressing save '{}' to '{}'", save.get_name(),
                     archive_path);

        std::vector<fs::path> files{save.get_save_path()};
        auto skse_cosave_path = save.get_skse_cosave_path();

        if (fs::exists(skse_cosave_path)) {
            files.emplace_back(skse_cosave_path);
        }

        pool.enqueue([&ok, files, archive_path] {
            if (!tools::zip_files(files, archive_path)) {
                ok = false;
            }
        });
    }

    pool.wait();
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
    auto compressed_saves_files =
        tools::list_files(fs::path(conf.backup_path) / character_id);

    std::vector<fs::path> compressed_saves;
    for (const auto &file : compressed_saves_files) {
        auto filename(file.filename().string());
        if (!filename.starts_with("Save") || !filename.ends_with(".zip")) {
            continue;
        }

        if (Save::parse_character_id(filename).empty()) {
            continue;
        }

        compressed_saves.emplace_back(file);
    }

    if (compressed_saves.size() <= conf.max_backed_up_saves) {
        return true;
    }

    std::ranges::sort(compressed_saves,
                      [](const fs::path &a, const fs::path &b) {
                          return Save::parse_number(a.stem().string()) >
                                 Save::parse_number(b.stem().string());
                      });

    bool ok = true;
    std::for_each(compressed_saves.begin() + conf.max_backed_up_saves,
                  compressed_saves.end(), [&ok](const fs::path &save) {
                      SPDLOG_INFO("Removing file '{}'", save.string());
                      if (!fs::remove(save)) {
                          SPDLOG_ERROR("Failed to remove file '{}'",
                                       save.string());
                          ok = false;
                      }
                  });
    return ok;
}

std::vector<Save> list_character_saves(const std::string &character_id) {
    auto files = tools::list_files(conf.saves_path);
    std::vector<Save> saves;

    for (const auto &file : files) {
        auto save = Save::from_save_path(file);

        if (!save.has_value() || save->get_character_id() != character_id) {
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