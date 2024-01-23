#include "tools.hpp"

namespace fs = std::filesystem;

namespace tools {

std::vector<fs::path> list_files_from_directory(const std::string& directory) {
    std::vector<fs::path> ret;
    for (const auto& dir_entry : fs::directory_iterator(directory)) {
        ret.emplace_back(dir_entry.path());
    }
    return ret;
}

} // namespace tools