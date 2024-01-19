#include <filesystem>
#include "spdlog/sinks/basic_file_sink.h"

#include "plugin_init.hpp"
#include "Conf.hpp"
#include "savetools.hpp"

namespace fs = std::filesystem;

void message_handler(SKSE::MessagingInterface::Message* message) {
    if (message->type == SKSE::MessagingInterface::kSaveGame) {
        savetools::manage_saves(static_cast<char *>(message->data));
    }
}

extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Load(const SKSE::LoadInterface* skse) {
    setup_log();
    SKSE::Init(skse);

    const Conf& conf = Conf::get_singleton();
    if (!fs::exists(conf.saves_path)) {
        SPDLOG_ERROR("Save path '{}' does not exist, will do nothing", conf.saves_path);
        return true;
    }

    if (conf.compress && !fs::exists(conf.backup_path)) {
        fs::create_directories(conf.backup_path);
    }

    SKSE::GetMessagingInterface()->RegisterListener(message_handler);
    return true;
}