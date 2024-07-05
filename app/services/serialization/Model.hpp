#pragma once

#include <vector>
#include <map>

#include "Property.hpp"

class base_model;

class base_serializer {
protected:
    std::map<std::string, base_property*>& get_properties(base_model& model) const;
    std::vector<std::pair<std::string, base_property*>>& get_sorted_properties(base_model& model) const;
};

class base_model {
protected:
    friend class base_property;
    friend class base_serializer;

    std::map<std::string, base_property*> properties;
    std::vector<std::pair<std::string, base_property*>> sorted_properties;
};
