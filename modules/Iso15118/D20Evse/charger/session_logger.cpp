// SPDX-License-Identifier: Apache-2.0
// Copyright Pionix GmbH and Contributors to EVerest
#include "session_logger.hpp"

#include <chrono>
#include <cstdio>
#include <fstream>
#include <stdexcept>

#include <time.h>

#include <iso15118/session/logger.hpp>

using LogEvent = iso15118::session::logging::Event;

std::string get_filename_for_current_time() {
    const auto now = std::chrono::system_clock::now();
    const auto now_t = std::chrono::system_clock::to_time_t(now);

    std::tm now_tm;
    localtime_r(&now_t, &now_tm);

    char buffer[64];
    strftime(buffer, sizeof(buffer), "%y%m%d_%H-%M-%S.yaml", &now_tm);
    return buffer;
}

class SessionLog {
public:
    SessionLog(const std::string& file_name) : file(file_name.c_str(), std::ios::out) {
        if (not file.good()) {
            throw std::runtime_error("Failed to open file " + file_name + " for writing iso15118 session log");
        }

        printf("Created logfile at: %s\n", file_name.c_str());
    }
    void operator()(const iso15118::session::logging::SimpleEvent& event) {
        file << "  " << event.info << "\n";
    }

    void operator()(const iso15118::session::logging::ExiMessageEvent& event) {
    }

    void flush() {
        file.flush();
    }

private:
    std::fstream file;
};

SessionLogger::SessionLogger(std::filesystem::path output_dir_) : output_dir(std::filesystem::absolute(output_dir_)) {
    // FIXME (aw): this is quite brute force ...
    if (not std::filesystem::exists(output_dir)) {
        std::filesystem::create_directory(output_dir);
    }

    iso15118::session::logging::set_session_log_callback([this](std::uintptr_t id, const LogEvent& event) {
        auto log_it = logs.find(id);
        if (log_it == logs.end()) {
            const auto log_file_name = output_dir / get_filename_for_current_time();
            const auto emplaced = logs.emplace(id, std::make_unique<SessionLog>(log_file_name.string()));

            log_it = emplaced.first;
        }

        auto& log = *log_it->second;

        std::visit(log, event);
        log.flush();
    });
}

SessionLogger::~SessionLogger() = default;