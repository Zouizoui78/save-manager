#pragma once

#include <mutex>
#include <thread>

#include "spdlog/spdlog.h"

class Conf final {
public:
    // Settings
    std::string saves_path{""};
    std::string backup_path{""};
    bool backup{false};
    uint32_t n_saves_to_keep{10};
    uint32_t max_backed_up_saves{1000};

    // API
    static const Conf &get_singleton();
    static bool is_loaded();

    // Deleted constructors should be public
    // to get better compiler messages
    Conf(const Conf &other) = delete;
    Conf(Conf &&) = delete;
    Conf &operator=(const Conf &) = delete;
    Conf &operator=(Conf &&) = delete;

private:
    Conf() = default;
    ~Conf() = default;

    static void load(Conf &singleton);
    static bool _conf_loaded;
    static std::mutex _mutex;
};
