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

#include "Response.hpp"

enum route_verb {
    verb_get,
    verb_post,
    verb_put,
    verb_delete,
    verb_unknown = -1
};

extern std::map<std::string, std::function<std::any(std::string)>> value_mappers;

extern void route_get   (std::string uri, std::function<std::unique_ptr<response>(std::map<std::string, std::any>)> callback);
extern void route_post  (std::string uri, std::function<std::unique_ptr<response>(std::map<std::string, std::any>)> callback);
extern void route_put   (std::string uri, std::function<std::unique_ptr<response>(std::map<std::string, std::any>)> callback);
extern void route_delete(std::string uri, std::function<std::unique_ptr<response>(std::map<std::string, std::any>)> callback);
extern void router();
extern void init_router();
extern void init_routes();

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
