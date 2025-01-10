// SPDX-License-Identifier: Apache-2.0
// Copyright 2022 - 2024 Pionix GmbH and Contributors to EVerest

#include "tutorial_filterImpl.hpp"

namespace module {
namespace main {

void tutorial_filterImpl::init() {
    uptime_reduction = mod->config.config_filter_every_nths;
}

void tutorial_filterImpl::ready() {
    //mod->r_interface_tutorial_module->subscribe_var_nprocesses(cb_nproc_receive);
    mod->r_interface_tutorial_module->subscribe_var_nprocesses([&](int nproc) {
        std::lock_guard<std::mutex> lock(cv_m_nproc);
        q_nproc.push(nproc);
        cv_nproc.notify_one();
    });
    mod->r_interface_tutorial_module->subscribe_var_uptime([&](int uptime) {
        std::lock_guard<std::mutex> lock(cv_m_uptime);
        q_uptime.push(uptime);
        cv_uptime.notify_one();
    });

    std::thread t_uptime(std::bind(&tutorial_filterImpl::process_uptime_updates, this));
    std::thread t_nproc(std::bind(&tutorial_filterImpl::process_nproc_updates, this));

    t_uptime.join();
    t_nproc.join();
}

std::string tutorial_filterImpl::handle_command_set_mode_odd(bool& chooseOdd) {
    // your code for cmd command_set_mode_odd goes here
    mode_odd = chooseOdd;
    if (mode_odd) {
        return "odd";
    } else {
        return "even";
    }
}

void tutorial_filterImpl::process_uptime_updates() {
    int sample_count = 0;

    while (true) {
        std::unique_lock<std::mutex> lock(cv_m_uptime);
        while (q_uptime.empty()) {
            cv_uptime.wait(lock);
        }

        int new_uptime = q_uptime.front();
        q_uptime.pop();

        sample_count += 1;

        if (sample_count == uptime_reduction) {
            sample_count = 0;
            publish_var_filtered_uptime(new_uptime);
        }
    }
}

void tutorial_filterImpl::process_nproc_updates() {
    int old_nproc {0};
    while (true) {
        std::unique_lock<std::mutex> lock(cv_m_nproc);
        while (q_nproc.empty()) {
            cv_nproc.wait(lock);
        }

        int new_nproc = q_nproc.front();
        q_nproc.pop();

        if (new_nproc == old_nproc) {
            continue;
        }

        if ( (mode_odd && (new_nproc % 2 == 1)) || (!mode_odd && (new_nproc % 2 == 0)) ) {
            publish_var_filtered_nproc(new_nproc);
            old_nproc = new_nproc;
        }
    }
}

} // namespace main
} // namespace module
