#include "zip.hpp"
#include "zip.h" // libzip

#include "spdlog/spdlog.h"
#include "spdlog/stopwatch.h"

namespace fs = std::filesystem;

bool zip_files_add(const std::vector<fs::path> &paths, zip_t *zipfile) {
    if (zipfile == nullptr) {
        SPDLOG_ERROR("Can't add files, zipfile = nullptr");
        return false;
    }

    bool no_error = true;
    for (const auto &path : paths) {
        SPDLOG_DEBUG("Trying to add {} to archive...", path.filename().string());

        zip_source_t *source = zip_source_file(zipfile, path.string().c_str(), 0, 0);
        if (source == nullptr) {
            SPDLOG_ERROR("Failed to create source from {} : {}", path.filename().string(), zip_strerror(zipfile));
            no_error = false;
            continue;
        }

        int64_t ret = zip_file_add(zipfile, path.filename().string().c_str(), source, ZIP_FL_OVERWRITE);
        if (ret == -1) {
            SPDLOG_ERROR("Failed to add {} : {}", path.filename().string(), zip_strerror(zipfile));
            no_error = false;
        }

        SPDLOG_DEBUG("Added {}", path.filename().string());
    }

    return no_error;
}

void zip_callback(zip_t *zipfile, double progress, void *arg) {
    (void)zipfile;
    (void)arg;
    if (progress == 0) {
        return;
    }
    SPDLOG_INFO("Compression progress : {:.0f}%", progress * 100);
}

void zip_files(const std::vector<std::filesystem::path>& files, const std::string& dest) {
    spdlog::stopwatch sw;

    zip_t *zipfile = zip_open(dest.c_str(), ZIP_CREATE, nullptr);
    if (zipfile == nullptr) {
        SPDLOG_ERROR("Failed to open {}", dest);
        exit(1);
    }

    zip_register_progress_callback_with_state(zipfile, 0.1, &zip_callback, nullptr, nullptr);
    zip_files_add(files, zipfile);

    SPDLOG_INFO("Compressing files...");
    if (zip_close(zipfile) == -1) {
        SPDLOG_ERROR("Failed to write and close archive : {}", zip_strerror(zipfile));
    }
    SPDLOG_INFO("Done compressing in {:.3f}s", sw);
}