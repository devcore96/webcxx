#pragma once

#include "AST.hpp"

#include <cstddef>
#include <map>
#include <variant>
#include <concepts>
#include <sstream>

#include "Model.hpp"

namespace json {
    class json_value;

    class json {
    private:
        std::map<std::string, json_value> values;

    public:
        json()             = default;
        json(const json& ) = default;
        json(      json&&) = default;

        json& operator=(const json& ) = default;
        json& operator=(      json&&) = default;

        json& operator=(const json_value& );
        json& operator=(      json_value&&);

        operator std::string();
        json_value& operator[](std::string key);

        void   set(std::string key, json_value value);
        void unset(std::string key) { values.erase(key); }

        std::map<std::string, json_value>::        iterator  begin();
        std::map<std::string, json_value>::        iterator    end();
        std::map<std::string, json_value>::reverse_iterator rbegin();
        std::map<std::string, json_value>::reverse_iterator   rend();

        std::string beautify(size_t layer = 0);
    };

    class json_value {
    public:
        enum type_t { basic, object, array, unset };

    private:
        type_t type;
        std::variant<std::string, json, std::vector<json_value>> value;

    public:
        json_value()                                          : type(unset),   value(std::string("")) { }

        // null
        json_value(std::nullptr_t null)                       : type(basic),   value(std::string("null")) { }

        // string
        json_value(std::string string)                        : type(basic),   value(std::string("\"") +                         string  + "\"") { }
        json_value(const char* string)                        : type(basic),   value(std::string("\"") +                         string  + "\"") { }
        json_value(      char* string)                        : type(basic),   value(std::string("\"") + const_cast<const char*>(string) + "\"") { }

        // bool
        json_value(bool value)                                : type(basic),   value(std::string(value ? "true" : "false")) { }

        // number
        json_value(std::integral       auto i)                : type(basic),   value(std::to_string(i)) { }
        json_value(std::floating_point auto f)                : type(basic),   value(std::to_string(f)) { }

        // object
        json_value(json json_object)                          : type(object),  value(json_object) { }

        // array
        json_value(std::vector<json_value> values)            : type(array),   value(values) { }

        // null
        json_value& operator=(std::nullptr_t null)            { type = basic;  value = std::string("null");                                        return *this; }

        // string
        json_value& operator=(std::string string)             { type = basic;  value = std::string("\"") +                         string  + "\""; return *this; }
        json_value& operator=(const char* string)             { type = basic;  value = std::string("\"") +                         string  + "\""; return *this; }
        json_value& operator=(      char* string)             { type = basic;  value = std::string("\"") + const_cast<const char*>(string) + "\""; return *this; }

        // bool
        json_value& operator=(bool bool_value)                { type = basic;  value = std::string(bool_value ? "true" : "false");                 return *this; }

        // number
        json_value& operator=(std::integral       auto i)     { type = basic;  value = std::to_string(i);                                          return *this; }
        json_value& operator=(std::floating_point auto f)     { type = basic;  value = std::to_string(f);                                          return *this; }

        // object
        json_value& operator=(json json_object)               { type = object; auto placeholder = std::variant<std::string, json, std::vector<json_value>> { json_object }; value.swap(placeholder); return *this; }

        // array
        json_value& operator=(std::vector<json_value> values) { type = array;  value = values;                                                     return *this; }

        bool operator==(std::nullptr_t            val) { return type == basic    && std::get<std::string>(value) == "null"; }
        bool operator==(std::string               val) { return type == basic    && std::get<std::string>(value) ==  val; }
        bool operator==(const char*               val) { return type == basic    && std::get<std::string>(value) ==  val; }
        bool operator==(      char*               val) { return type == basic    && std::get<std::string>(value) ==  val; }
        bool operator==(bool                      val) { return type == basic    && std::get<std::string>(value) == (val ? "true" : "false"); }
        bool operator==(json                      val) { return type == object   &&        ((std::string) *this) == ((std::string)val); }
        bool operator==(std::vector<json_value>   val) { return type == array    &&        ((std::string) *this) == ((std::string)json_value { val }); }
        bool operator==(json_value                val) { return type == val.type &&        ((std::string) *this) == ((std::string)val); }
        bool operator==(std::integral       auto  val) { if    (type != basic) return false; std::istringstream ss(std::get<std::string>(value)); auto _value = (decltype(val))0; ss >> _value; return _value == val; }
        bool operator==(std::floating_point auto  val) { if    (type != basic) return false; std::istringstream ss(std::get<std::string>(value)); auto _value = (decltype(val))0; ss >> _value; return _value == val; }

        bool isset() { return type != unset; }

        operator std::string() {
            switch (type) {
                case basic: return std::get<std::string>(value);
                case object: return (std::string)std::get<json>(value);
                case array:
                    auto& values = std::get<std::vector<json_value>>(value);

                    if (values.size() == 0) return "[ ]";

                    std::string ret = "[ ";

                    for (auto& value : values)
                        ret += (std::string)value + ", ";

                    ret = ret.substr(0, ret.length() - 2) + " ]";

                    return ret;
            }

            return "";
        }

        std::string unescaped() {
            std::string ret = *this;

            if (ret[0] == '"') {
                return ret.substr(1, ret.size() - 2);
            }

            return ret;
        }

        operator json() const {
            if (type != object) {
                throw std::runtime_error("This JSON value is not an object");
            }

            return std::get<json>(value);
        }

        json_value& operator[](size_t key) {
            if (type == unset) {
                type = array;
                value = std::vector<json_value> { };
            }

            if (type != array)
                throw std::runtime_error("This JSON value is not an array");

            auto& vector = std::get<std::vector<json_value>>(value);

            if (key >= vector.size()) {
                vector.resize(key + 1, json_value { });
            }

            return vector[key];
        }

        json_value& operator[](std::string key) {
            if (type == unset) {
                type = object;
                value = json { };
            }

            if (type != object)
                throw std::runtime_error("This JSON value is not an object");

            return std::get<json>(value)[key];
        }

        std::vector<json_value>::iterator begin() {
            if (type == unset) {
                type = array;
                value = std::vector<json_value> { };
            }

            if (type != array)
                throw std::runtime_error("This JSON value is not an array");

            return std::get<std::vector<json_value>>(value).begin();
        }

        std::vector<json_value>::iterator end() {
            if (type == unset) {
                type = array;
                value = std::vector<json_value> { };
            }

            if (type != array)
                throw std::runtime_error("This JSON value is not an array");

            return std::get<std::vector<json_value>>(value).end();
        }

        std::vector<json_value>::reverse_iterator rbegin() {
            if (type == unset) {
                type = array;
                value = std::vector<json_value> { };
            }

            if (type != array)
                throw std::runtime_error("This JSON value is not an array");

            return std::get<std::vector<json_value>>(value).rbegin();
        }

        std::vector<json_value>::reverse_iterator rend() {
            if (type == unset) {
                type = array;
                value = std::vector<json_value> { };
            }

            if (type != array)
                throw std::runtime_error("This JSON value is not an array");

            return std::get<std::vector<json_value>>(value).rend();
        }

        std::string beautify(size_t layer = 0) {
            switch (type) {
                case basic: return std::get<std::string>(value);
                case object: return std::get<json>(value).beautify(layer);
                case array:
                    auto values = std::get<std::vector<json_value>>(value);

                    if (values.size() == 0) return "[ ]";

                    std::string ret = "[\n";

                    std::string tabs = "";

                    for (size_t i = 0; i < layer; i++) {
                        tabs += "    ";
                    }

                    for (auto& value : values) {
                        ret += tabs + "    " + value.beautify(layer + 1) + ",\n";
                    }

                    ret = ret.substr(0, ret.length() - 2) + '\n' + tabs + "]";

                    return ret;
            }

            return "";
        }

        type_t get_type() { return type; }
    };

    std::ostream& operator<<(std::ostream& lhs, json& rhs);
    std::ostream& operator<<(std::ostream& lhs, json_value& rhs);

    using json_token = std::variant<
        int,
                              std::string,
        std::vector<std::pair<std::string, json_value>>,
                    std::pair<std::string, json_value>,
        std::vector<                       json_value >,
                                           json_value
    >;

    class parser : public ast::basic_parser<json_token, char> {
    private:
        enum {
            // basic tokens
            object_begin,
            object_end,
            array_begin,
            array_end,
            comma,
            colon,
            value_true,
            value_false,
            value_null,
            value_string,
            value_number,

            // start rule (either object or array)
            start,

            // basic rules
            object,
            value_pairs,
            value_pair,
            value,
            array,
            values
        };

        // Helper class used to convert json_token to json value
        class json_token_visitor {
        public:
            json_value operator()(std::vector<json_value> value) { return json_value{ value }; }
            json_value operator()(            json_value  value) { return             value;   }

            // These 4 will never occur, however they are needed for std::visit
            json_value operator()(int) {
                throw std::runtime_error("JSON parser unexpected error: start token was int");
            }

            json_value operator()(std::string) {
                throw std::runtime_error("JSON parser unexpected error: start token was std::string");
            }

            json_value operator()(std::vector<std::pair<std::string, json_value>>) {
                throw std::runtime_error("JSON parser unexpected error: start token was std::vector<std::pair<std::string, json_value>>");
            }

            json_value operator()(std::pair<std::string, json_value>) {
                throw std::runtime_error("JSON parser unexpected error: start token was std::pair<std::string, json_value>");
            }
        };

    public:
        parser() : ast::basic_parser<json_token, char>({
            // skip whitespaces
            { std::regex("^[ \t\n\r]+"),
                [](std::string str, json_token& token) {                                                       return std::nullopt; } },

            // json tokens
            { std::string("{"),
                [](std::string str, json_token& token) {                                                       return object_begin; } },
            { std::string("}"),
                [](std::string str, json_token& token) {                                                       return object_end;   } },
            { std::string("["),
                [](std::string str, json_token& token) {                                                       return array_begin;  } },
            { std::string("]"),
                [](std::string str, json_token& token) {                                                       return array_end;    } },
            { std::string(","),
                [](std::string str, json_token& token) {                                                       return comma;        } },
            { std::string(":"),
                [](std::string str, json_token& token) {                                                       return colon;        } },
            { std::string("true"),
                [](std::string str, json_token& token) { token = json_value { true };                          return value_true;   } },
            { std::string("false"),
                [](std::string str, json_token& token) { token = json_value { false };                         return value_false;  } },
            { std::string("null"),
                [](std::string str, json_token& token) { token = json_value { nullptr };                       return value_null;   } },
            { std::regex("^\".*?\""),
                [](std::string str, json_token& token) { token = json_value { str.substr(1, str.size() - 2) }; return value_string; } },
            { std::regex("^[-+]?[0-9]+\\.?[0-9]*(?:[Ee][-+]?[0-9]+)?"),
                [](std::string str, json_token& token) { token = json_value { std::stod(str) };                return value_number; } }
        }, {
            // json rules
            { start,       { { { object },
                [](std::vector<json_token> tokens) -> json_token { return tokens[0]; } },
                             { { array },
                [](std::vector<json_token> tokens) -> json_token { return tokens[0]; } } } },
            { object,      { { { object_begin, object_end },
                [](std::vector<json_token> tokens) -> json_token { return json_value { json { } }; } },
                             { { object_begin, value_pairs, object_end },
                [](std::vector<json_token> tokens) -> json_token {
                    json object { };

                    auto& pairs = std::get<std::vector<std::pair<std::string, json_value>>>(tokens[1]);

                    for (auto& pair : pairs)
                        object.set(pair.first, pair.second);

                    return json_value { object };
                } } } },
            { value_pairs, { { { value_pair, comma, value_pairs },
                [](std::vector<json_token> tokens) -> json_token {
                    auto pairs = std::get<std::vector<std::pair<std::string, json_value>>>(tokens[2]);

                    pairs.push_back(std::get<std::pair<std::string, json_value>>(tokens[0]));

                    return pairs;
                } },
                             { { value_pair },
                [](std::vector<json_token> tokens) -> json_token {
                    std::vector<std::pair<std::string, json_value>> pairs;

                    pairs.push_back(std::get<std::pair<std::string, json_value>>(tokens[0]));

                    return pairs;
                } } } },
            { value_pair,  { { { value_string, colon, value },
                [](std::vector<json_token> tokens) -> json_token {
                    std::string key = std::get<json_value>(tokens[0]);
                    key = key.substr(1, key.size() - 2);
                    return std::make_pair(
                        key,
                        std::get<json_value>(tokens[2])
                    );
                } } } },
            { value,       { { { value_true },
                [](std::vector<json_token> tokens) -> json_token { return tokens[0]; } },
                             { { value_false },
                [](std::vector<json_token> tokens) -> json_token { return tokens[0]; } },
                             { { value_null },
                [](std::vector<json_token> tokens) -> json_token { return tokens[0]; } },
                             { { value_string },
                [](std::vector<json_token> tokens) -> json_token { return tokens[0]; } },
                             { { value_number },
                [](std::vector<json_token> tokens) -> json_token { return tokens[0]; } },
                             { { object },
                [](std::vector<json_token> tokens) -> json_token { return tokens[0]; } },
                             { { array },
                [](std::vector<json_token> tokens) -> json_token { return tokens[0]; } } } },
            { array,       { { { array_begin, array_end },
                [](std::vector<json_token> tokens) -> json_token { return json_value { std::vector<json_value> { } }; } },
                             { { array_begin, values, array_end },
                [](std::vector<json_token> tokens) -> json_token { return json_value { std::get<std::vector<json_value>>(tokens[1]) }; } } } },
            { values,      { { { value, comma, values },
                [](std::vector<json_token> tokens) -> json_token {
                    auto values = std::get<std::vector<json_value>>(tokens[2]);

                    values.push_back(std::get<json_value>(tokens[0]));

                    return values;
                } },
                             { { value },
                [](std::vector<json_token> tokens) -> json_token {
                    std::vector<json_value> values;

                    values.push_back(std::get<json_value>(tokens[0]));

                    return values;
                } } } }
        }, start) { }

        json_value parse(std::string string, bool silent = false) {
            return std::visit(json_token_visitor { }, do_parse(std::regex_replace(std::regex_replace(string, std::regex("[\n\r]"), " "), std::regex("\\\\\\\""), "''"), silent));
        }
    };

    class serializer : public base_serializer {
    private:
        parser p;

    public:
        json   serialize(base_model& model);
        void deserialize(base_model& model, json object);
    };
}
