// SPDX-License-Identifier: Apache-2.0
// Copyright Pionix GmbH and Contributors to EVerest

#include "mapping_reader.hpp"
#include "everest/logging.hpp"
#include "util.hpp"
#include <iostream>

namespace module {
OcppToEverestConfigMapping MappingReader::read_mapping(const std::filesystem::path& file_path) {
    const auto hacked_file_path =
        std::filesystem::path{"modules"} / "OCPPConfiguration" / file_path; // TODO: this is very hacky

    const auto tree = Util::load_existing_user_config(hacked_file_path);
    const auto root = tree.rootref();

    return parse_mapping(root);
}
OcppToEverestConfigMapping MappingReader::parse_mapping(const c4::yml::NodeRef& root) {
    auto mapping = OcppToEverestConfigMapping{};

    for (const auto& mapping_node : root) {
        auto mapped_node = parse_mapping_node(mapping_node);
        mapping.insert(std::move(mapped_node));
    }
    return mapping;
}
std::pair<ComponentVariable, EverestConfigMapping>
MappingReader::parse_mapping_node(const c4::yml::NodeRef& mapping_node) {
    auto component_variable = parse_component_variable_node(mapping_node);
    auto module_mapping = parse_maps_to_node(mapping_node["maps_to"]);
    return {std::move(component_variable), std::move(module_mapping)};
}

ComponentVariable MappingReader::parse_component_variable_node(const c4::yml::NodeRef& node) {
    // component is optional
    auto component = node.has_child("component") ? parse_component_node(node["component"]) : Component{};

    // variable is required
    auto variable = parse_variable_node(node["variable"]);
    return {std::move(component), std::move(variable)};
}

Component MappingReader::parse_component_node(const c4::yml::NodeRef& node) {
    auto evse = node.has_child("evse") ? parse_evse_node(node["evse"]) : std::nullopt;

    const auto instance_optinal_val = node.has_child("instance") ? std::optional{node["instance"].val()} : std::nullopt;
    auto instance = instance_optinal_val.has_value()
                        ? std::optional{std::string{instance_optinal_val.value().str, instance_optinal_val.value().len}}
                        : std::nullopt;

    auto component_name = node.has_child("name") ? std::string{node["name"].val().str, node["name"].val().len} : "";
    return {std::move(component_name), std::move(instance), std::move(evse)};
}

Variable MappingReader::parse_variable_node(const c4::yml::NodeRef node) {
    const auto node_has_name = node.has_child("name");
    if (!node_has_name) {
        throw std::runtime_error("Variable node must have a name");
    }

    const auto name_has_val = node["name"].has_val();
    if (!name_has_val) {
        throw std::runtime_error("Variable name must have a value");
    }

    const auto variable_name_val = node["name"].val();
    auto variable_name = std::string{variable_name_val.str, variable_name_val.len};

    const auto instance_optional_val =
        node.has_child("instance") ? std::optional{node["instance"].val()} : std::nullopt;
    auto instance =
        instance_optional_val.has_value()
            ? std::optional{std::string{instance_optional_val.value().str, instance_optional_val.value().len}}
            : std::nullopt;
    return {std::move(variable_name), std::move(instance)};
}

std::optional<types::ocpp::EVSE> MappingReader::parse_evse_node(const c4::yml::NodeRef node) {
    if (!node.has_child("id")) {
        throw std::runtime_error("EVSE node must have an id");
    }
    const auto id_val = node["id"].val();
    auto id = std::stoi(std::string{id_val.str, id_val.len});

    const auto connector_id_optional_val = node.has_child("id") ? std::optional{node["id"].val()} : std::nullopt;
    auto connector_id = connector_id_optional_val.has_value()
                            ? std::optional{std::stoi(std::string{connector_id_optional_val.value().str,
                                                                  connector_id_optional_val.value().len})}
                            : std::nullopt;

    return types::ocpp::EVSE{std::move(id), std::move(connector_id)};
}

EverestConfigMapping MappingReader::parse_maps_to_node(const ryml::NodeRef& node) {

    const auto parse_node = [](const auto& node) {
        const auto val = node.val();
        return std::string{val.str, val.len};
    };

    const auto module_id_node = node["module_id"];
    const auto config_key_node = node["config_key"];

    const auto module_id = parse_node(module_id_node);
    const auto config_key = parse_node(config_key_node);

    return {module_id, config_key};
}

} // namespace module