// SPDX-License-Identifier: Apache-2.0
// Copyright Pionix GmbH and Contributors to EVerest
#include "Tutorial_FilterModule.hpp"

namespace module {

void Tutorial_FilterModule::init() {
    invoke_init(*p_main);
}

void Tutorial_FilterModule::ready() {
    invoke_ready(*p_main);
}

} // namespace module
