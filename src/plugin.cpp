#include <filesystem>
#include <spdlog/sinks/basic_file_sink.h>

#include "Conf.hpp"
#include "savetools.hpp"
#include "wintools.hpp"

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

SKSEPluginLoad(const SKSE::LoadInterface *skse) {
    SKSE::Init(skse);
    setup_log();

    SKSE::GetMessagingInterface()->RegisterListener([](SKSE::MessagingInterface::Message *message) {
        const Conf& conf = Conf::get_singleton();
        if (message->type == SKSE::MessagingInterface::kDataLoaded) {
            SPDLOG_INFO("saves path : {}", conf.saves_path);
            for (const auto &save : list_saves()) {
                SPDLOG_INFO("{}", save.filename().string());
            }
            SPDLOG_INFO("");
        }
        if (message->type == SKSE::MessagingInterface::kSaveGame) {
            // TODO plugin impl
            for (const auto &save : list_saves()) {
                SPDLOG_INFO("{}", save.filename().string());
            }
            SPDLOG_INFO("");
        }
    });

    return true;
}