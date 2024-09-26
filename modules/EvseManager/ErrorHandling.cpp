// SPDX-License-Identifier: Apache-2.0
// Copyright 2023 Pionix GmbH and Contributors to EVerest

#include "ErrorHandling.hpp"

namespace module {

using ErrorList = std::list<Everest::error::ErrorType>;
static const struct IgnoreErrors {
    // p_evse. We need to ignore Inoperative here as this is the result of this check.
    ErrorList evse{"evse_manager/Inoperative"};
    ErrorList bsp{"evse_board_support/MREC3HighTemperature", "evse_board_support/MREC18CableOverTempDerate",
                  "evse_board_support/VendorWarning"};
    ErrorList connector_lock{"connector_lock/VendorWarning"};
    ErrorList ac_rcd{"ac_rcd/VendorWarning"};
    ErrorList imd{"isolation_monitor/VendorWarning"};
    ErrorList powersupply{"power_supply_DC/VendorWarning"};
} ignore_errors;

ErrorHandling::ErrorHandling(const std::unique_ptr<evse_board_supportIntf>& _r_bsp,
                             const std::vector<std::unique_ptr<ISO15118_chargerIntf>>& _r_hlc,
                             const std::vector<std::unique_ptr<connector_lockIntf>>& _r_connector_lock,
                             const std::vector<std::unique_ptr<ac_rcdIntf>>& _r_ac_rcd,
                             const std::unique_ptr<evse_managerImplBase>& _p_evse,
                             const std::vector<std::unique_ptr<isolation_monitorIntf>>& _r_imd,
                             const std::vector<std::unique_ptr<power_supply_DCIntf>>& _r_powersupply) :
    r_bsp(_r_bsp),
    r_hlc(_r_hlc),
    r_connector_lock(_r_connector_lock),
    r_ac_rcd(_r_ac_rcd),
    p_evse(_p_evse),
    r_imd(_r_imd),
    r_powersupply(_r_powersupply) {

    // Subscribe to bsp driver to receive Errors from the bsp hardware
    r_bsp->subscribe_all_errors([this](const Everest::error::Error& error) { process_error(); },
                                [this](const Everest::error::Error& error) { process_error(); });

    // Subscribe to connector lock to receive errors from connector lock hardware
    if (r_connector_lock.size() > 0) {
        r_connector_lock[0]->subscribe_all_errors([this](const Everest::error::Error& error) { process_error(); },
                                                  [this](const Everest::error::Error& error) { process_error(); });
    }

    // Subscribe to ac_rcd to receive errors from AC RCD hardware
    if (r_ac_rcd.size() > 0) {
        r_ac_rcd[0]->subscribe_all_errors([this](const Everest::error::Error& error) { process_error(); },
                                          [this](const Everest::error::Error& error) { process_error(); });
    }

    // Subscribe to ac_rcd to receive errors from IMD hardware
    if (r_imd.size() > 0) {
        r_imd[0]->subscribe_all_errors([this](const Everest::error::Error& error) { process_error(); },
                                       [this](const Everest::error::Error& error) { process_error(); });
    }

    // Subscribe to powersupply to receive errors from DC powersupply hardware
    if (r_powersupply.size() > 0) {
        r_powersupply[0]->subscribe_all_errors([this](const Everest::error::Error& error) { process_error(); },
                                               [this](const Everest::error::Error& error) { process_error(); });
    }
}

void ErrorHandling::raise_overcurrent_error(const std::string& description) {
    // raise externally
    Everest::error::Error error_object = p_evse->error_factory->create_error(
        "evse_manager/MREC4OverCurrentFailure", "", description, Everest::error::Severity::High);
    p_evse->raise_error(error_object);
    process_error();
}

void ErrorHandling::clear_overcurrent_error() {
    // clear externally
    if (p_evse->error_state_monitor->is_error_active("evse_manager/MREC4OverCurrentFailure", "")) {
        p_evse->clear_error("evse_manager/MREC4OverCurrentFailure", "");
    }
    process_error();
}

// Find out if the current error set is fatal to charging or not
void ErrorHandling::process_error() {
    const auto fatal = errors_prevent_charging();
    if (fatal) {
        // signal to charger a new error has been set that prevents charging
        raise_inoperative_error(*fatal);
    } else {
        // signal an error that does not prevent charging
        clear_inoperative_error();
    }

    // All errors cleared signal is for OCPP 1.6. It is triggered when there are no errors anymore,
    // even those that did not block charging.

    auto number_of_active_errors = [](const auto& impl) {
        if (impl.size() > 0) {
            return static_cast<int>(impl[0]->error_state_monitor->get_active_errors().size());
        } else {
            return 0;
        }
    };

    const int error_count = p_evse->error_state_monitor->get_active_errors().size() +
                            r_bsp->error_state_monitor->get_active_errors().size() +
                            number_of_active_errors(r_connector_lock) + number_of_active_errors(r_ac_rcd) +
                            number_of_active_errors(r_imd) + number_of_active_errors(r_powersupply);

    if (error_count == 0) {
        signal_all_errors_cleared();
    }
}

namespace detail {
template <typename T> struct ErrorsToCheck {
    T& unit;
    const ErrorList& errors_to_ignore;
};

template <typename T> ErrorsToCheck(T&, const ErrorList&) -> ErrorsToCheck<T>;

std::optional<std::string> get_first_fatal_error(const std::list<Everest::error::ErrorPtr>& active_errors,
                                                 const ErrorList& errors_to_ignore) {
    for (const auto& error : active_errors) {
        if (std::none_of(std::begin(errors_to_ignore), std::end(errors_to_ignore),
                         [&error](const auto& error_to_ignore) { return error->type == error_to_ignore; })) {
            return error->type;
        }
    }

    return std::nullopt;
}

template <typename T> auto get_first_fatal_error(ErrorsToCheck<T>& set) {
    return get_first_fatal_error(set.unit.error_state_monitor->get_active_errors(), set.errors_to_ignore);
}

// specialization for units contained in a vector
template <typename T> std::optional<std::string> get_first_fatal_error(ErrorsToCheck<const std::vector<T>>& set) {
    for (const auto& item : set.unit) {
        const auto fatal = get_first_fatal_error(ErrorsToCheck{*item, set.errors_to_ignore});
        if (fatal) {
            return fatal;
        }
    }

    return std::nullopt;
}

template <typename... Ts> auto get_first_fatal_error(ErrorsToCheck<Ts>... sets) {
    std::optional<std::string> fatal;
    // the following line makes use of short-circuiting in boolean expression
    (((fatal = get_first_fatal_error(sets)) || ...));
    return fatal;
}
} // namespace detail

// Check all errors from p_evse and all requirements to see if they block charging
std::optional<std::string> ErrorHandling::errors_prevent_charging() {
    /* clang-format off */
    return get_first_fatal_error(
        detail::ErrorsToCheck{*p_evse, ignore_errors.evse},
        detail::ErrorsToCheck{*r_bsp, ignore_errors.bsp},
        detail::ErrorsToCheck{r_connector_lock, ignore_errors.connector_lock},
        detail::ErrorsToCheck{r_ac_rcd, ignore_errors.ac_rcd},
        detail::ErrorsToCheck{r_imd, ignore_errors.imd},
        detail::ErrorsToCheck{r_imd, ignore_errors.imd}
    );
    /* clang-format on */
}

void ErrorHandling::raise_inoperative_error(const std::string& caused_by) {
    if (p_evse->error_state_monitor->is_error_active("evse_manager/Inoperative", "")) {
        // dont raise if already raised
        return;
    }

    if (r_hlc.size() > 0) {
        r_hlc[0]->call_send_error(types::iso15118_charger::EvseError::Error_Malfunction);
    }

    // raise externally
    Everest::error::Error error_object =
        p_evse->error_factory->create_error("evse_manager/Inoperative", "", caused_by, Everest::error::Severity::High);
    p_evse->raise_error(error_object);

    signal_error(true);
}

void ErrorHandling::clear_inoperative_error() {
    // clear externally
    if (p_evse->error_state_monitor->is_error_active("evse_manager/Inoperative", "")) {
        p_evse->clear_error("evse_manager/Inoperative");
        signal_error(false);
    }
}

void ErrorHandling::raise_internal_error(const std::string& description) {
    // raise externally
    Everest::error::Error error_object =
        p_evse->error_factory->create_error("evse_manager/Internal", "", description, Everest::error::Severity::High);
    p_evse->raise_error(error_object);
    process_error();
}

void ErrorHandling::clear_internal_error() {
    // clear externally
    if (p_evse->error_state_monitor->is_error_active("evse_manager/Internal", "")) {
        p_evse->clear_error("evse_manager/Internal");
        process_error();
    }
}

void ErrorHandling::raise_powermeter_transaction_start_failed_error(const std::string& description) {
    // raise externally
    Everest::error::Error error_object = p_evse->error_factory->create_error(
        "evse_manager/PowermeterTransactionStartFailed", "", description, Everest::error::Severity::High);
    p_evse->raise_error(error_object);
    process_error();
}

void ErrorHandling::clear_powermeter_transaction_start_failed_error() {
    // clear externally
    if (p_evse->error_state_monitor->is_error_active("evse_manager/PowermeterTransactionStartFailed", "")) {
        p_evse->clear_error("evse_manager/PowermeterTransactionStartFailed");
        process_error();
    }
}

} // namespace module
