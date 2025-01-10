// SPDX-License-Identifier: Apache-2.0
// Copyright Pionix GmbH and Contributors to EVerest
#include "Tutorial_SourceModule.hpp"

namespace module {

void Tutorial_SourceModule::init() {
    invoke_init(*p_main);
}

void Tutorial_SourceModule::ready() {
    invoke_ready(*p_main);
}

} // namespace module
