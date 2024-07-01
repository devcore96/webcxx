#pragma once

#include <map>

#include "Property.hpp"

class base_model;

class base_serializer {
protected:
    std::map<std::string, base_property*>& get_properties(base_model& model) const;
    serialized get_value(base_property* property) const;
    void set_value(base_property* property, serialized value) const;
};

class base_model {
protected:
    friend class base_property;
    friend class base_serializer;

    std::map<std::string, base_property*> properties;
};
