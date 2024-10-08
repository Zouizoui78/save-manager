#pragma once

#ifdef SKYRIM_SUPPORT_AE
extern "C" DLLEXPORT constinit auto SKSEPlugin_Version = []() {
    SKSE::PluginVersionData v;
    v.PluginVersion(
        REL::Version{version::MAJOR, version::MINOR, version::PATCH});
    v.PluginName(version::PROJECT);
    v.AuthorName("zoui");
    v.UsesAddressLibrary();
    v.UsesUpdatedStructs();
    v.CompatibleVersions({SKSE::RUNTIME_LATEST});
    return v;
}();
#else
extern "C" DLLEXPORT bool SKSEAPI
SKSEPlugin_Query(const SKSE::QueryInterface *a_skse, SKSE::PluginInfo *a_info) {
    a_info->infoVersion = SKSE::PluginInfo::kVersion;
    a_info->name = version::PROJECT;
    a_info->version = version::MAJOR;

    if (a_skse->IsEditor()) {
        SKSE::log::critical("Loaded in editor, marking as incompatible");
        return false;
    }

    const auto ver = a_skse->RuntimeVersion();
    if (ver < SKSE::RUNTIME_1_5_39) {
        SKSE::log::critical(FMT_STRING("Unsupported runtime version {}"),
                            ver.string());
        return false;
    }

    return true;
}
#endif

void setup_log() {
    auto logsFolder = SKSE::log::log_directory();
    if (!logsFolder) {
        SKSE::stl::report_and_fail(
            "SKSE log_directory not provided, logs disabled.");
        return;
    }

    auto logFilePath = *logsFolder / std::format("{}.log", version::PROJECT);
    auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(
        logFilePath.string(), true);
    auto logger = std::make_shared<spdlog::logger>("log", std::move(sink));

    logger->set_level(spdlog::level::info);
    logger->flush_on(spdlog::level::info);

    spdlog::set_default_logger(std::move(logger));
}