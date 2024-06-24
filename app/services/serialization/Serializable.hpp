#pragma once

#include <string>
#include <variant>
#include <concepts>

template<class T>
concept natively_serializable = std::integral<T> ||
                                std::floating_point<T> ||
                                std::same_as<bool, T> ||
                                std::same_as<std::nullptr_t, T>;

template<class T>
concept to_string_serializable = !natively_serializable<T> &&
                                 (std::convertible_to<std::string, T> ||
                                  std::constructible_from<std::string, T>);

template<class T>
concept serializable = natively_serializable<T> || to_string_serializable<T>;

using serialized_t = std::variant<long long,
                                  long double,
                                  bool,
                                  std::nullptr_t,
                                  std::string>;

struct serialized {
    enum {
        integer,
        floating_point,
        boolean,
        null,
        string
    } type;
    serialized_t value;
};
