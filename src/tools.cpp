#include "tools.hpp"

namespace fs = std::filesystem;

namespace tools {

std::vector<fs::path>
list_files_from_directory(const fs::path &directory,
                          const std::vector<std::string> &filters) {
    std::vector<fs::path> ret;
    for (const auto &dir_entry : fs::directory_iterator(directory)) {
        auto &path = dir_entry.path();
        auto path_str = path.string();

        bool filter_ok = true;
        if (!filters.empty()) {
            filter_ok = false;
            for (const auto &filter : filters) {
                if (path_str.contains(filter)) {
                    filter_ok = true;
                    break;
                }
            }
        }

        if (filter_ok) {
            ret.emplace_back(path);
        }
    }
    return ret;
}

} // namespace tools