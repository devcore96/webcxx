#include "Json.hpp"

#include <sstream>
#include <stdexcept>

namespace json {
    json_value array() {
        std::vector<json_value> values;

        return { values };
    }

    std::ostream& operator<<(std::ostream& lhs, json& rhs) {
        lhs << rhs.beautify();
        return lhs;
    }

    std::ostream& operator<<(std::ostream& lhs, json_value& rhs) {
        lhs << rhs.beautify();
        return lhs;
    }

    json::operator std::string() {
        std::string str = "{ ";

        for (auto& value_pair : values)
            str += "\"" + value_pair.first + "\": " + (value_pair.second.operator std::string()) + ", ";

        // Remove excess ", " from the end
        if (str.length() > 2)
            str = str.substr(0, str.length() - 2) + " ";

        str += "}";

        return str;
    }

    json_value& json::operator[](std::string key) {
        if (!values.contains(key)) {
            values.insert(std::make_pair(key, json_value{ }));
        }

        return values[key];
    }

    void json::set(std::string key, json_value value) { values.emplace(key, value); }

    std::string json::beautify(size_t layer) {
        std::string str = "{\n";

        std::string tabs = "";

        for (size_t i = 0; i < layer; i++) {
            tabs += "    ";
        }

        for (auto& value_pair : values)
            str += tabs + "    \"" + value_pair.first + "\": " + value_pair.second.beautify(layer + 1) + ",\n";

        // Remove excess "," from the end
        if (str.length() > 2)
            str = str.substr(0, str.length() - 2) + '\n' + tabs;

        str += "}";

        return str;
    }

    std::map<std::string, json_value>::        iterator json:: begin() { return values. begin(); }
    std::map<std::string, json_value>::        iterator json::   end() { return values.   end(); }
    std::map<std::string, json_value>::reverse_iterator json::rbegin() { return values.rbegin(); }
    std::map<std::string, json_value>::reverse_iterator json::  rend() { return values.  rend(); }

    json& json::operator=(const json_value&  value) { return *this = value.operator json(); }
    json& json::operator=(      json_value&& value) { return *this = value.operator json(); }

    json serializer::serialize(base_model& model) const {
        auto& properties = get_properties(model);

        json object;

        for(auto& key_value_pair : properties) {
            auto value = key_value_pair.second->serialize_value();

            switch(value.type) {
                case serialized::integer:        object[key_value_pair.first] = std::get<long long     >(value.value); break;
                case serialized::floating_point: object[key_value_pair.first] = std::get<long double   >(value.value); break;
                case serialized::boolean:        object[key_value_pair.first] = std::get<bool          >(value.value); break;
                case serialized::null:           object[key_value_pair.first] = std::get<std::nullptr_t>(value.value); break;
                case serialized::string:         object[key_value_pair.first] = std::get<std::string   >(value.value); break;
            }
        }

        return object;
    }

    void serializer::deserialize(base_model& model, json object) const {
        auto& properties = get_properties(model);

        for(auto& key_value_pair : properties) {
            auto value = key_value_pair.second->serialize_value();

            auto& json_val = object[key_value_pair.first];

            if (json_val.get_type() != json_value::basic) throw std::bad_cast();

            switch(value.type) {
                case serialized::integer:        { std::istringstream stream (json_val.unescaped()); long long   val = 0; stream >> val; value.value = val;                  break; }
                case serialized::floating_point: { std::istringstream stream (json_val.unescaped()); long double val = 0; stream >> val; value.value = val;                  break; }
                case serialized::boolean:                                                                                                value.value = json_val == true;     break;
                case serialized::null:                                                                                                   value.value = nullptr;              break;
                case serialized::string:                                                                                                 value.value = json_val.unescaped(); break;
            }

            key_value_pair.second->deserialize_value(value);
        }
    }

}
