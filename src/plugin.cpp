#include <filesystem>
#include <spdlog/sinks/basic_file_sink.h>

#include "Conf.hpp"
#include "savetools.hpp"

namespace logger = SKSE::log;
namespace fs = std::filesystem;

extern "C" DLLEXPORT constinit auto SKSEPlugin_Version = []() {
    SKSE::PluginVersionData v;
    v.PluginVersion(1);
    v.PluginName("save-manager");
    v.AuthorName("zoui");
    v.CompatibleVersions({0});
    return v;
}();

extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Query(const SKSE::QueryInterface* a_skse, SKSE::PluginInfo* a_info)
{
    a_info->infoVersion = SKSE::PluginInfo::kVersion;
    a_info->name = "save-manager";
    a_info->version = 1;

    if (a_skse->IsEditor()) {
        logger::critical("Loaded in editor, marking as incompatible"sv);
        return false;
    }

    const auto ver = a_skse->RuntimeVersion();
    if (ver < SKSE::RUNTIME_1_5_39) {
        logger::critical(FMT_STRING("Unsupported runtime version {}"), ver.string());
        return false;
    }

    return true;
}

void setup_log() {
    auto logsFolder = SKSE::log::log_directory();
    if (!logsFolder) {
        SKSE::stl::report_and_fail("SKSE log_directory not provided, logs disabled.");
        return;
    }
    auto pluginName = "save-manager";
    auto logFilePath = *logsFolder / std::format("{}.log", pluginName);
    auto fileLoggerPtr = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logFilePath.string(), true);
    auto loggerPtr = std::make_shared<spdlog::logger>("log", std::move(fileLoggerPtr));
    loggerPtr->set_level(spdlog::level::trace);
    spdlog::set_default_logger(std::move(loggerPtr));
}

void message_handler(SKSE::MessagingInterface::Message* message) {
    if (message->type == SKSE::MessagingInterface::kSaveGame) {
        savetools::manage_saves(static_cast<char *>(message->data));
    }
}

extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Load(const SKSE::LoadInterface* skse) {
    SKSE::Init(skse);
    setup_log();

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