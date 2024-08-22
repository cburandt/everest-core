// SPDX-License-Identifier: Apache-2.0
// Copyright Pionix GmbH and Contributors to EVerest

#include "event_handler.hpp"
#include "util.hpp"
#include <everest/logging.hpp>

namespace module {

EventHandler::EventHandler(const std::filesystem::path& config_mapping_path) :
    config_mapping(MappingReader::read_mapping(config_mapping_path)) {
}

void EventHandler::handle_event(const types::ocpp::EventData& event_data, const std::string& user_config_path_string) {

    const auto& everest_module_mapping_opt =
        find_mapping_by_component_variable_or_log_error(event_data.component_variable);
    if (!everest_module_mapping_opt.has_value()) {
        return;
    }

    const auto& everest_module_mapping = everest_module_mapping_opt.value();

    write_event_to_config(event_data, user_config_path_string, everest_module_mapping);
}
void EventHandler::write_event_to_config(const types::ocpp::EventData& event_data,
                                         const std::string& user_config_path_string,
                                         const EverestConfigMapping& everest_module_mapping) {
    const auto user_config_path = std::filesystem::path{user_config_path_string};
    auto tree = Util::load_existing_user_config(user_config_path);
    Util::write_value_to_tree(everest_module_mapping, event_data.actual_value, tree);
    Util::save_tree_to_yaml_file(tree, user_config_path);
}

const std::optional<EverestConfigMapping> EventHandler::find_mapping_by_component_variable_or_log_error(
    const types::ocpp::ComponentVariable& component_variable) const {
    try {
        const auto& everest_module_mapping = find_mapping_by_component_variable(component_variable);
        return everest_module_mapping;
    } catch (const std::runtime_error& e) {
        EVLOG_error << e.what();
        return std::nullopt;
    }
}

const EverestConfigMapping
EventHandler::find_mapping_by_component_variable(const types::ocpp::ComponentVariable& component_variable) const {
    const auto& mapping = config_mapping.find(component_variable);

    // check if event is in event_map
    if (mapping == config_mapping.end()) {
        throw std::runtime_error("Component Variable not found in config mapping:\n Component:\t" +
                                 std::string(component_variable.component.name) + "\n Variable:\t" +
                                 std::string(component_variable.variable.name));
    }

    return mapping->second;
}

} // namespace module
