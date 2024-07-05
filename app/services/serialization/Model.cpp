#include "Model.hpp"

std::map<std::string, base_property*>& base_serializer::get_properties(base_model& model) const {
    return model.properties;
}

std::vector<std::pair<std::string, base_property*>>& base_serializer::get_sorted_properties(base_model& model) const {
    return model.sorted_properties;
}

void base_property::register_property() {
    model->properties.insert(std::make_pair(key, this));
    model->sorted_properties.push_back(std::make_pair(key, this));
}
