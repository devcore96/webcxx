#include "Model.hpp"

std::map<std::string, base_property*>& base_serializer::get_properties(base_model& model) const {
    return model.properties;
}

serialized base_serializer::get_value(base_property* property) const {
    return property->serialize_value();
}

void base_serializer::set_value(base_property* property, serialized value) const {
    property->deserialize_value(value);
}

void base_property::register_property() {
    model->properties.insert(std::make_pair(key, this));
}
