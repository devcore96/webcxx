#include "Model.hpp"

std::map<std::string, base_property*>& base_serializer::get_properties(base_model& model) {
    return model.properties;
}

serialized base_serializer::get_value(base_property* property) {
    return property->serialize_value();
}

void base_serializer::set_value(base_property* property, serialized value) {
    property->deserialize_value(value);
}

void base_property::register_property() {
    model->properties.insert(std::make_pair(key, this));
}
