#include "spdlog/sinks/basic_file_sink.h"
#include <filesystem>

#include "Conf.hpp"
#include "plugin_init.hpp"
#include "savetools.hpp"

namespace fs = std::filesystem;

void message_handler(SKSE::MessagingInterface::Message *message) {
    if (message->type == SKSE::MessagingInterface::kSaveGame) {
        savetools::manage_saves(static_cast<char *>(message->data));
    }
}

extern "C" DLLEXPORT bool SKSEAPI
SKSEPlugin_Load(const SKSE::LoadInterface *skse) {
    setup_log();
    SKSE::Init(skse);

    const Conf &conf = Conf::get_singleton();
    if (!fs::exists(conf.saves_path)) {
        SPDLOG_ERROR("Save path '{}' does not exist, will do nothing",
                     conf.saves_path);
        return true;
    }

    SKSE::GetMessagingInterface()->RegisterListener(message_handler);
    spdlog::info("Plugin successfully initialized");
    return true;
}