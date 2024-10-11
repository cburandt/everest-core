#include <ReservationEVSEs.h>

#include <algorithm>
#include <iostream>

namespace module {
ReservationEVSEs::ReservationEVSEs() {
}

void ReservationEVSEs::add_connector(const uint32_t evse_id, const uint32_t connector_id,
                                     const types::evse_manager::ConnectorTypeEnum connector_type,
                                     const ConnectorState connector_state) {
    EvseConnectorType evse_connector_type;
    evse_connector_type.connector_type = connector_type;
    evse_connector_type.connector_id = connector_id;
    evse_connector_type.state = connector_state;
    evses[evse_id].push_back(evse_connector_type);
    max_scenarios.clear();
    max_scenarios = create_scenarios();
    // create({}, nullptr);
    print_scenarios(max_scenarios);

    // if (max_scenarios.size() == 0) {
    //     max_scenarios.push_back(Scenario());
    // }

    // bool added = false;
    // for (auto& evse : max_scenarios.at(0).evse) {
    //     if (evse.evse_id == evse_id) {
    //         // TODO mz we assume add_connector is called once per connector, document!!
    //         evse.connector_type.push_back(connector_type);
    //         added = true;
    //         break;
    //     }
    // }

    // if (!added) {
    //     EVSE_Connectors c;
    //     c.evse_id = evse_id;

    //     c.connector_type.push_back(connector_type);
    //     max_scenarios.at(0).evse.push_back(c);
    // }

    // // TODO mz create current scenarios and remove occupied, faulted and unavailable connectors / evses
    // current_scenarios = max_scenarios;

    // print_scenarios(max_scenarios);

    // create_scenarios();
}

bool ReservationEVSEs::make_reservation(std::optional<uint32_t> evse_id,
                                        const types::evse_manager::ConnectorTypeEnum connector_type) {
    if (evse_id.has_value()) {
        // TODO mz
    } else {
        std::vector<types::evse_manager::ConnectorTypeEnum> types = global_reservations;
        types.push_back(connector_type);
        std::vector<std::vector<types::evse_manager::ConnectorTypeEnum>> orders = get_all_possible_orders(types);

        for (const auto& possible_order : orders) {
            if (!is_scenario_available(possible_order)) {
                return false;
            }
        }
    }

    // make_new_current_scenario(evse_id, connector_type);
    global_reservations.push_back(connector_type);

    return true;
}

void ReservationEVSEs::print_scenarios(const std::vector<Scenario> scenarios) {
    uint32_t scenario_count = 0;
    for (const auto& scenario : scenarios) {
        std::cout << "\n=====\n Scenario: " << scenario_count++ << std::endl;
        for (const auto& evse : scenario.evse_connector) {
            std::cout << "\n  EVSE: " << evse.evse_id << ", Connector: " << evse.connector_type << std::endl;
        }
    }
    std::cout << "\n---------------------\n";
}

std::vector<ReservationEVSEs::Scenario> ReservationEVSEs::create_scenarios() {

    std::vector<ReservationEVSEs::Scenario> scenario;
    using namespace types::evse_manager;
    // 1A 2A 3A
    scenario =
        add_scenario(scenario, 0, ConnectorTypeEnum::cCCS2, 1, ConnectorTypeEnum::cCCS2, 2, ConnectorTypeEnum::cCCS2);
    // 1A 2A 3B
    scenario =
        add_scenario(scenario, 0, ConnectorTypeEnum::cCCS2, 1, ConnectorTypeEnum::cCCS2, 2, ConnectorTypeEnum::cType2);
    // 1A 2B 3A
    scenario =
        add_scenario(scenario, 0, ConnectorTypeEnum::cCCS2, 1, ConnectorTypeEnum::cType2, 2, ConnectorTypeEnum::cCCS2);
    // 1A 2B 3B
    scenario =
        add_scenario(scenario, 0, ConnectorTypeEnum::cCCS2, 1, ConnectorTypeEnum::cType2, 2, ConnectorTypeEnum::cType2);

    // 1A 3A 2A
    scenario =
        add_scenario(scenario, 0, ConnectorTypeEnum::cCCS2, 2, ConnectorTypeEnum::cCCS2, 1, ConnectorTypeEnum::cCCS2);
    // 1A 3B 2A
    scenario =
        add_scenario(scenario, 0, ConnectorTypeEnum::cCCS2, 2, ConnectorTypeEnum::cType2, 1, ConnectorTypeEnum::cCCS2);
    // 1A 3A 2B
    scenario =
        add_scenario(scenario, 0, ConnectorTypeEnum::cCCS2, 2, ConnectorTypeEnum::cCCS2, 1, ConnectorTypeEnum::cType2);
    // 1A 3B 2B
    scenario =
        add_scenario(scenario, 0, ConnectorTypeEnum::cCCS2, 2, ConnectorTypeEnum::cType2, 1, ConnectorTypeEnum::cType2);

    // 2A 1A 3A
    scenario =
        add_scenario(scenario, 1, ConnectorTypeEnum::cCCS2, 0, ConnectorTypeEnum::cCCS2, 2, ConnectorTypeEnum::cCCS2);
    // 2A 1A 3B
    scenario =
        add_scenario(scenario, 1, ConnectorTypeEnum::cCCS2, 0, ConnectorTypeEnum::cCCS2, 2, ConnectorTypeEnum::cType2);
    // 2A 3A 1A
    scenario =
        add_scenario(scenario, 1, ConnectorTypeEnum::cCCS2, 2, ConnectorTypeEnum::cCCS2, 0, ConnectorTypeEnum::cCCS2);
    // 2A 3B 1A
    scenario =
        add_scenario(scenario, 1, ConnectorTypeEnum::cCCS2, 2, ConnectorTypeEnum::cType2, 0, ConnectorTypeEnum::cCCS2);

    // 2B 1A 3A
    scenario =
        add_scenario(scenario, 1, ConnectorTypeEnum::cType2, 0, ConnectorTypeEnum::cCCS2, 2, ConnectorTypeEnum::cCCS2);
    // 2B 1A 3B
    scenario =
        add_scenario(scenario, 1, ConnectorTypeEnum::cType2, 0, ConnectorTypeEnum::cCCS2, 2, ConnectorTypeEnum::cType2);
    // 2B 3A 1A
    scenario =
        add_scenario(scenario, 1, ConnectorTypeEnum::cType2, 2, ConnectorTypeEnum::cCCS2, 0, ConnectorTypeEnum::cCCS2);
    // 2B 3B 1A
    scenario =
        add_scenario(scenario, 1, ConnectorTypeEnum::cType2, 2, ConnectorTypeEnum::cType2, 0, ConnectorTypeEnum::cCCS2);

    // 3A 1A 2A
    scenario =
        add_scenario(scenario, 2, ConnectorTypeEnum::cCCS2, 0, ConnectorTypeEnum::cCCS2, 1, ConnectorTypeEnum::cCCS2);
    // 3A 1A 2B
    scenario =
        add_scenario(scenario, 2, ConnectorTypeEnum::cCCS2, 0, ConnectorTypeEnum::cCCS2, 1, ConnectorTypeEnum::cType2);
    // 3A 2A 1A
    scenario =
        add_scenario(scenario, 2, ConnectorTypeEnum::cCCS2, 1, ConnectorTypeEnum::cCCS2, 0, ConnectorTypeEnum::cCCS2);
    // 3A 2B 1A
    scenario =
        add_scenario(scenario, 2, ConnectorTypeEnum::cCCS2, 1, ConnectorTypeEnum::cType2, 0, ConnectorTypeEnum::cCCS2);

    // 3B 1A 2A
    scenario =
        add_scenario(scenario, 2, ConnectorTypeEnum::cType2, 0, ConnectorTypeEnum::cCCS2, 1, ConnectorTypeEnum::cCCS2);
    // 3B 1A 2B
    scenario =
        add_scenario(scenario, 2, ConnectorTypeEnum::cType2, 0, ConnectorTypeEnum::cCCS2, 1, ConnectorTypeEnum::cType2);
    // 3B 2A 1A
    scenario =
        add_scenario(scenario, 2, ConnectorTypeEnum::cType2, 1, ConnectorTypeEnum::cCCS2, 0, ConnectorTypeEnum::cCCS2);
    // 3B 2B 1A
    scenario =
        add_scenario(scenario, 2, ConnectorTypeEnum::cType2, 1, ConnectorTypeEnum::cType2, 0, ConnectorTypeEnum::cCCS2);

    // for (const auto& [evse_id, connectors] : evses) {
    //     for (const auto connector : connectors) {
    //         Scenario scenario;
    //         EVSE_Connector c;
    //         c.connector_type = connector;
    //         c.evse_id = evse_id;
    //         scenario.evse_connector.push_back(c);
    //         for (const auto& [evse_id_inner, connectors_inner] : evses) {
    //             if (evse_id == evse_id_inner) {
    //                 continue;
    //             }
    //             for (const auto connector_inner : connectors_inner) {
    //                 EVSE_Connector c;
    //                 c.evse_id = evse_id_inner;
    //                 c.connector_type = connector_inner;
    //             }
    //         }

    //         max_scenarios.push_back(scenario);
    //     }
    // }

    return scenario;
}

void ReservationEVSEs::make_new_current_scenario(std::optional<uint32_t> evse_id,
                                                 const types::evse_manager::ConnectorTypeEnum connector_type) {
    // if (evse_id != std::nullopt) {
    //     // TODO mz
    // } else {
    //     std::vector<Scenario> new_scenarios;
    //     for (const auto& scenario : current_scenarios) {
    //         for (const auto& evse : scenario.evse) {
    //             // if (!has_evse_connector_type(evse, connector_type)) {
    //             //     continue;
    //             // }
    //             Scenario new_scenario;
    //             for (const auto& other_evse : scenario.evse) {
    //                 if (other_evse.evse_id == evse.evse_id) {
    //                     continue;
    //                 }

    //                 new_scenario.evse.push_back(other_evse);
    //             }
    //             new_scenarios.push_back(new_scenario);
    //         }
    //     }

    //     current_scenarios = new_scenarios;
    // }

    // std::cout << "\n\n new current scenario: ----------\n";
    // print_scenarios(current_scenarios);
}

// bool ReservationEVSEs::has_evse_connector_type(const EVSE_Connectors& evse,
//                                                const types::evse_manager::ConnectorTypeEnum connector_type) {
//     // for (const auto& type : evse.connector_type) {
//     //     if (type == connector_type) {
//     //         return true;
//     //     }
//     // }

//     // return false;
// }

void ReservationEVSEs::create(std::vector<uint32_t> evse_ids, Scenario* scenario) {
    for (const auto& [evse_id, connector_types] : evses) {
        if (!evse_ids.empty() && std::find(evse_ids.begin(), evse_ids.end(), evse_id) != evse_ids.end()) {
            continue;
        }
        std::vector<uint32_t> ids = evse_ids;
        // ids.push_back(evse_id);

        for (const auto& connector_type : connector_types) {
            bool add_scenario = false;
            Scenario s;
            if (scenario == nullptr) {
                scenario = &s;
                add_scenario = true;
            } /*else {
                s = *scenario;
            }*/

            EVSE_Connector c;
            c.evse_id = evse_id;
            c.connector_type = connector_type.connector_type;
            // std::vector<uint32_t> ids = evse_ids;
            ids.push_back(evse_id);
            scenario->evse_connector.push_back(c);
            create(ids, scenario);

            if (add_scenario) {
                max_scenarios.push_back(*scenario);
            }
        }
    }
}

std::vector<ReservationEVSEs::Scenario>
ReservationEVSEs::add_scenario(std::vector<Scenario> scenarios, const uint32_t evse_id1,
                               const types::evse_manager::ConnectorTypeEnum connector_type1, const uint32_t evse_id2,
                               const types::evse_manager::ConnectorTypeEnum connector_type2, const uint32_t evse_id3,
                               const types::evse_manager::ConnectorTypeEnum connector_type3) {
    Scenario s;
    EVSE_Connector c;
    c.connector_type = connector_type1;
    c.evse_id = evse_id1;

    EVSE_Connector c2;
    c2.connector_type = connector_type2;
    c2.evse_id = evse_id2;

    EVSE_Connector c3;
    c3.evse_id = evse_id3;
    c3.connector_type = connector_type3;

    s.evse_connector.push_back(c);
    s.evse_connector.push_back(c2);
    s.evse_connector.push_back(c3);

    scenarios.push_back(s);
    return scenarios;
}

bool ReservationEVSEs::is_scenario_available(std::vector<types::evse_manager::ConnectorTypeEnum> connectors) {
    bool result = false;
    for (const auto& scenario : max_scenarios) {
        if (scenario.evse_connector.size() < connectors.size()) {
            // TODO mz Return false? Because all scenarios should have the same size.
            continue;
        }

        uint32_t connectors_fit = 1;
        if (scenario.evse_connector.at(0).connector_type != connectors.at(0)) {
            continue;
        }

        bool scenario_possible = false;

        for (uint32_t i = 1; i < connectors.size(); ++i) {
            if (scenario.evse_connector.at(i).connector_type != connectors.at(i)) {
                // TODO mz is evse id needed here? Because it has another connector available as well and we need to
                // know if both options are not ok or if there is still one option (it is an or).
                if (i + 1 == connectors.size() - 1) {
                    return false;
                }
            }
            connectors_fit++;
        }

        if (connectors_fit == connectors.size()) {
            scenario_possible = true;
            result = true;
        }
    }

    return result;
}

std::vector<std::string> get_order_string(std::string input) {
    std::string i = input;
    std::vector<std::string> order;
    do {
        order.push_back(i);
    } while (std::next_permutation(i.begin(), i.end()));

    order.push_back(i);

    return order;
}

std::vector<std::vector<types::evse_manager::ConnectorTypeEnum>>
ReservationEVSEs::get_all_possible_orders(std::vector<types::evse_manager::ConnectorTypeEnum> connectors) {
    std::vector<std::string> o = get_order_string("aba");

    std::vector<types::evse_manager::ConnectorTypeEnum> c = connectors;
    std::vector<std::vector<types::evse_manager::ConnectorTypeEnum>> result;
    do {
        result.push_back(c);
    } while (std::next_permutation(c.begin(), c.end()));

    // if (connectors.size() > 1) {
    //     result.push_back(c);
    // }

    return result;
}
} // namespace module
