#include <filesystem>
#include <spdlog/sinks/basic_file_sink.h>

#include "Conf.hpp"
#include "savetools.hpp"

namespace logger = SKSE::log;
namespace fs = std::filesystem;

void setup_log() {
    auto logsFolder = SKSE::log::log_directory();
    if (!logsFolder) {
        SKSE::stl::report_and_fail("SKSE log_directory not provided, logs disabled.");
        return;
    }
    auto pluginName = SKSE::PluginDeclaration::GetSingleton()->GetName();
    auto logFilePath = *logsFolder / std::format("{}.log", pluginName);
    auto fileLoggerPtr = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logFilePath.string(), true);
    auto loggerPtr = std::make_shared<spdlog::logger>("log", std::move(fileLoggerPtr));
    loggerPtr->set_level(spdlog::level::trace);
    spdlog::set_default_logger(std::move(loggerPtr));
}

void message_handler(SKSE::MessagingInterface::Message* message) {
    if (message->type == SKSE::MessagingInterface::kSaveGame) {
        manage_saves();
    }
}

SKSEPluginLoad(const SKSE::LoadInterface* skse) {
    SKSE::Init(skse);
    setup_log();

    const Conf& conf = Conf::get_singleton();
    if (!fs::exists(conf.saves_path)) {
        SPDLOG_ERROR("Save path '{}' does not exist, will do nothing", conf.saves_path);
        return true;
    }

    if (conf.compress) {
        fs::create_directories(conf.backup_path);
    }

    SKSE::GetMessagingInterface()->RegisterListener(message_handler);
    return true;
}