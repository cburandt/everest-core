// SPDX-License-Identifier: Apache-2.0
// Copyright Pionix GmbH and Contributors to EVerest
#ifndef MAIN_TUTORIAL_FILTER_IMPL_HPP
#define MAIN_TUTORIAL_FILTER_IMPL_HPP

//
// AUTO GENERATED - MARKED REGIONS WILL BE KEPT
// template version 3
//

#include <generated/interfaces/tutorial_filter/Implementation.hpp>

#include "../Tutorial_FilterModule.hpp"

// ev@75ac1216-19eb-4182-a85c-820f1fc2c091:v1
// insert your custom include headers here
#include <condition_variable>
#include <queue>
#include <thread>
// ev@75ac1216-19eb-4182-a85c-820f1fc2c091:v1

namespace module {
namespace main {

struct Conf {};

class tutorial_filterImpl : public tutorial_filterImplBase {
public:
    tutorial_filterImpl() = delete;
    tutorial_filterImpl(Everest::ModuleAdapter* ev, const Everest::PtrContainer<Tutorial_FilterModule>& mod,
                        Conf& config) :
        tutorial_filterImplBase(ev, "main"), mod(mod), config(config){};

    // ev@8ea32d28-373f-4c90-ae5e-b4fcc74e2a61:v1
    // insert your public definitions here
    void cb_uptime_receive(int uptime);
    void cb_nproc_receive(int nproc);
    // ev@8ea32d28-373f-4c90-ae5e-b4fcc74e2a61:v1

protected:
    // command handler functions (virtual)
    virtual std::string handle_command_set_mode_odd(bool& chooseOdd) override;

    // ev@d2d1847a-7b88-41dd-ad07-92785f06f5c4:v1
    // insert your protected definitions here
    // ev@d2d1847a-7b88-41dd-ad07-92785f06f5c4:v1

private:
    const Everest::PtrContainer<Tutorial_FilterModule>& mod;
    const Conf& config;

    virtual void init() override;
    virtual void ready() override;

    // ev@3370e4dd-95f4-47a9-aaec-ea76f34a66c9:v1
    // insert your private definitions here
    void process_uptime_updates();
    void process_nproc_updates();

    std::queue<int> q_uptime;
    std::queue<int> q_nproc;
    int uptime_reduction;
    bool mode_odd{false};
    std::condition_variable cv_uptime;
    std::condition_variable cv_nproc;
    std::mutex cv_m_uptime;
    std::mutex cv_m_nproc;
    // ev@3370e4dd-95f4-47a9-aaec-ea76f34a66c9:v1
};

// ev@3d7da0ad-02c2-493d-9920-0bbbd56b9876:v1
// insert other definitions here
// ev@3d7da0ad-02c2-493d-9920-0bbbd56b9876:v1

} // namespace main
} // namespace module

#endif // MAIN_TUTORIAL_FILTER_IMPL_HPP
