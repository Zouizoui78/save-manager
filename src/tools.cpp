#include "tools.hpp"
#include "shlobj.h"
#include "shlwapi.h"
#include "spdlog/stopwatch.h"
#include "zip.h"

namespace fs = std::filesystem;

namespace tools {

bool zip_files_add(const std::vector<fs::path> &paths, zip_t *zipfile);

// API

std::vector<fs::path> list_files(const fs::path &directory) {
    std::vector<fs::path> ret;
    for (const auto &dir_entry : fs::directory_iterator(directory)) {
        if (dir_entry.is_regular_file()) {
            ret.emplace_back(dir_entry.path());
        }
    }
    return ret;
}

fs::path get_documents_path() {
    std::string path;

    PWSTR pPath = NULL;
    if (SHGetKnownFolderPath(FOLDERID_Documents, KF_FLAG_DEFAULT, NULL,
                             &pPath) == S_OK) {
        int wlen = lstrlenW(pPath);
        int len =
            WideCharToMultiByte(CP_ACP, 0, pPath, wlen, NULL, 0, NULL, NULL);
        if (len > 0) {
            path.resize(len + 1);
            WideCharToMultiByte(CP_ACP, 0, pPath, wlen, &path[0], len, NULL,
                                NULL);
            path[len] = '\\';
        }
        CoTaskMemFree(pPath);
    }

    return fs::path(path);
}

bool zip_files(const std::vector<std::filesystem::path> &files,
               const std::string &dest) {
    spdlog::stopwatch sw;

    zip_t *zipfile = zip_open(dest.c_str(), ZIP_CREATE, nullptr);
    if (zipfile == nullptr) {
        SPDLOG_ERROR("Failed to open {}", dest);
        return false;
    }

    bool ok = zip_files_add(files, zipfile);
    if (!ok) {
        zip_discard(zipfile);
        SPDLOG_ERROR(
            "Failed to add at least one file to archive, discarding changes");
        return false;
    }

    SPDLOG_INFO("Compressing files...");
    if (zip_close(zipfile) == -1) {
        SPDLOG_ERROR("Failed to write and close archive : {}",
                     zip_strerror(zipfile));
        return false;
    }
    SPDLOG_INFO("Done compressing in {:.3f}s", sw);
    return true;
}

// HELPERS

bool zip_files_add(const std::vector<fs::path> &paths, zip_t *zipfile) {
    if (zipfile == nullptr) {
        SPDLOG_ERROR("Can't add files, zipfile = nullptr");
        return false;
    }

    bool no_error = true;
    for (const auto &path : paths) {
        SPDLOG_DEBUG("Trying to add {} to archive...",
                     path.filename().string());

        zip_source_t *source =
            zip_source_file(zipfile, path.string().c_str(), 0, 0);
        if (source == nullptr) {
            SPDLOG_ERROR("Failed to create source from {} : {}",
                         path.filename().string(), zip_strerror(zipfile));
            no_error = false;
            continue;
        }

        int64_t ret = zip_file_add(zipfile, path.filename().string().c_str(),
                                   source, ZIP_FL_OVERWRITE);
        if (ret == -1) {
            SPDLOG_ERROR("Failed to add {} : {}", path.filename().string(),
                         zip_strerror(zipfile));
            no_error = false;
        }

        SPDLOG_DEBUG("Added {}", path.filename().string());
    }

    return no_error;
}

} // namespace tools