#include "Json.hpp"
#include <stdexcept>

namespace json {
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
}
