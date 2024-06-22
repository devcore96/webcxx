#pragma once

#include <memory>
#include <any>
#include <tuple>
#include <functional>
#include <iostream>
#include <sstream>
#include <vector>
#include <map>
#include <concepts>

#include "../rest/Response.hpp"

enum route_verb {
    verb_get,
    verb_post,
    verb_put,
    verb_patch,
    verb_delete,
    verb_options,
    verb_unknown = -1
};

struct route_data {
    std::vector<route_verb> verb;
    std::string uri;
    std::function<std::unique_ptr<response>(std::map<std::string, std::any>)> callback;
};

extern std::map<std::string, std::function<std::any(std::string)>> value_mappers;

extern void router();
extern void init_router();

template<typename T>
T try_string_cast(std::string value) {
    std::istringstream ss(value);

    T val;
    if ((ss >> val).fail() || !(ss >> std::ws).eof())
        throw std::bad_cast();

    return val;
}

template<typename T>
void map_value(std::string value_identifier) {
    value_mappers.insert({ value_identifier, try_string_cast<T> });
}

#include "RouteTypes.hpp"
