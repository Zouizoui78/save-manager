#include "Conf.hpp"
#include "savetools.hpp"

namespace fs = std::filesystem;

std::vector<fs::path> list_files_from_directory(const std::string &directory) {
    std::vector<fs::path> ret;
    for (const auto &dir_entry : fs::directory_iterator(directory)) {
        ret.emplace_back(dir_entry.path());
    }
    return ret;
}

std::vector<std::filesystem::path> list_saves() {
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
    return files;
}