// SPDX-License-Identifier: Apache-2.0
// Copyright 2022 - 2024 Pionix GmbH and Contributors to EVerest

#include "tutorial_systemImpl.hpp"

using namespace std::chrono_literals;

namespace module {
namespace main {

void tutorial_systemImpl::init() {
}

void tutorial_systemImpl::ready() {
    while(true) {
        std::this_thread::sleep_for(2000ms);
        updateSysInfo();
        if (info.uptime != old_uptime) {
            publish_var_uptime(info.uptime);
        }
        if (info.procs != old_nproc) {
            publish_var_nprocesses(info.procs);
        }
    }
}

std::string tutorial_systemImpl::handle_command_tutorial(std::string& payload) {
    // your code for cmd command_tutorial goes here
    return "everest";
}

void tutorial_systemImpl::updateSysInfo() {
    sysinfo(&info);  // omitting evaluation of return value and errno !
}

} // namespace main
} // namespace module
