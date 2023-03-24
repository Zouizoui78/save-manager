#include "Conf.hpp"
#include "wintools.hpp"

using json = nlohmann::json;

bool Conf::_conf_loaded = false;
std::mutex Conf::_mutex;

const Conf& Conf::get_singleton() {
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

void Conf::load(Conf& singleton) {
    singleton.saves_path = (get_documents_path() / "My Games" / "Skyrim Special Edition" / "Saves").string();

    std::ifstream input("Data/SKSE/Plugins/save-manager.json");

    // if no conf file use defaults
    if (!input.is_open()) {
        _conf_loaded = true;
        return;
    }

    json j = json::parse(input);

    if (j.contains("saves_path")) {
        singleton.saves_path = j.at("saves_path").get<std::string>();
    }

    if (j.contains("archive_path")) {
        singleton.archive_path = j.at("archive_path").get<std::string>();
    }

    if (j.contains("compress")) {
        singleton.compress = j.at("compress").get<bool>();
    }

    if (j.contains("n_saves_to_keep")) {
        singleton.n_saves_to_keep = j.at("n_saves_to_keep").get<uint16_t>();
    }

    if (j.contains("max_saves_in_archive")) {
        singleton.max_saves_in_archive = j.at("max_saves_in_archive").get<uint32_t>();
    }

    _conf_loaded = true;
}