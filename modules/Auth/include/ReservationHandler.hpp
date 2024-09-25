// SPDX-License-Identifier: Apache-2.0
// Copyright Pionix GmbH and Contributors to EVerest

#ifndef RESERVATION_HANDLER_HPP
#define RESERVATION_HANDLER_HPP

#include <vector>

#include <Connector.hpp>
#include <everest/timer.hpp>
#include <generated/types/reservation.hpp>
#include <utils/types.hpp>

namespace module {

class ReservationHandler {

private: // Members
    std::map<int, types::reservation::Reservation> reservations;
    std::vector<types::reservation::Reservation> global_reservations;
    std::map<int, types::evse_manager::ConnectorTypeEnum> connectors;

    std::mutex timer_mutex;
    std::mutex reservation_mutex;
    // std::map<int, std::unique_ptr<Everest::SteadyTimer>> connector_to_reservation_timeout_timer_map;
    std::map<int, std::unique_ptr<Everest::SteadyTimer>> reservation_id_to_reservation_timeout_timer_map;

    std::function<void(const int& connector_id)> reservation_cancelled_callback;

public:
    /**
     * @brief Initializes a connector with the given \p connector_id . This creates an entry in the map of timers of
     the
     * handler.
     *
     * @param connector_id
     * @param connector_type    The connector type
     */
    void init_connector(const int connector_id, const types::evse_manager::ConnectorTypeEnum connector_type);

    /**
     * @brief Function checks if the given \p id_token or \p parent_id_token matches the reserved token of the given \p
     * connector
     *
     * @param connector         Connector id
     * @param id_token          Id token
     * @param parent_id_token   Parent id token
     * @return The reservation id when there is a matching identifier, otherwise std::nullopt.
     */
    std::optional<int32_t> matches_reserved_identifier(int connector, const std::string& id_token,
                                                       std::optional<std::string> parent_id_token);

    /**
     * @brief Functions check if reservation at the given \p connector contains a parent_id
     * @param connector
     * @return true if reservation for \p connector exists and reservation contains a parent_id
     */
    bool has_reservation_parent_id(int connector);

    /**
     * @brief Function tries to reserve the given \p connector using the given \p reservation
     *
     * @param connector
     * @param state Current state of the connector
     * @param is_reservable
     * @param reservation
     * @param available_connectors  The number of connectors available for this connector type. Only needed when
     *                              connector id is 0.
     * @return types::reservation::ReservationResult
     */
    types::reservation::ReservationResult reserve(int connector, const ConnectorState& state, bool is_reservable,
                                                  const types::reservation::Reservation& reservation,
                                                  const std::optional<uint32_t> available_connectors);

    /**
     * @brief Function tries to cancel reservation with the given \p reservation_id .
     *
     * @param reservation_id
     * @param execute_callback if true, cancel_reservation_callback will be executed
     * @return int -1 if reservation could not been cancelled, else the id of the connnector
     */
    int cancel_reservation(int reservation_id, bool execute_callback);

    /**
     * @brief Handler that is called when a reservation was started / used.
     *
     * @param connector         Connector on which the car is charging
     * @param reservation_id    The reservation id
     */
    void on_reservation_used(int connector, const int32_t reservation_id);

    /**
     * @brief Registers the given \p callback that is called when a reservation should be cancelled.
     *
     * @param callback
     */
    void register_reservation_cancelled_callback(const std::function<void(const int& connector_id)>& callback);

private: // Functions
    bool is_connector_type_available(const types::evse_manager::ConnectorTypeEnum connector_type);
    void set_reservation_timer(const types::reservation::Reservation& reservation, const int32_t connector);
};

} // namespace module

#endif // RESERVATION_HANDLER_HPP
