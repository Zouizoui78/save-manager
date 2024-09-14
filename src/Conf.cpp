#include "Conf.hpp"
#include "nlohmann/json.hpp"
#include "wintools.hpp"

using json = nlohmann::json;

bool Conf::_conf_loaded = false;
std::mutex Conf::_mutex;

const Conf &Conf::get_singleton() {
    std::lock_guard<std::mutex> lock(_mutex);
    static Conf singleton;
    if (!is_loaded()) {
        load(singleton);
    }
    return singleton;
}

bool Conf::is_loaded() {
    return _conf_loaded;
}

void Conf::load(Conf &singleton) {
    std::filesystem::path documents_sse_path =
        wintools::get_documents_path() / "My Games" / "Skyrim Special Edition";
    singleton.saves_path = (documents_sse_path / "Saves").string();
    singleton.backup_path = (documents_sse_path / "save-manager").string();

    std::ifstream input("Data/SKSE/Plugins/save-manager.json");

    // if no conf file use defaults
    if (!input.is_open()) {
        _conf_loaded = true;
        return;
    }

    json j = json::parse(input);

    if (j.contains("saves_path") && !j.at("saves_path").empty()) {
        singleton.saves_path = j.at("saves_path").get<std::string>();
    }

    if (j.contains("backup_path") && !j.at("backup_path").empty()) {
        singleton.backup_path = j.at("backup_path").get<std::string>();
    }

    if (j.contains("backup")) {
        singleton.backup = j.at("backup").get<bool>();
    }

    if (j.contains("n_saves_to_keep")) {
        singleton.n_saves_to_keep = j.at("n_saves_to_keep").get<uint32_t>();
    }

    if (j.contains("max_backed_up_saves")) {
        singleton.max_backed_up_saves =
            j.at("max_backed_up_saves").get<uint32_t>();
    }

    _conf_loaded = true;
}