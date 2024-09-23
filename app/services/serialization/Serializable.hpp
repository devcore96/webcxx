#pragma once

#include <string>
#include <memory>
#include <variant>
#include <concepts>

#include "../tools/Container.hpp"

class base_model;

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
concept model_serializable = (std::convertible_to<std::shared_ptr<base_model>, T> ||
                              std::constructible_from<std::shared_ptr<base_model>, T> ||
                              std::same_as<std::shared_ptr<base_model>, T>) && !std::same_as<std::nullptr_t, T>;

template<class T>
concept model_container_serializable = !model_serializable<T> &&
                                        iterable<T> &&
                                        requires(T t) {
                                            requires model_serializable<decltype(*t.begin())>;
                                        };

template<class T>
concept serializable = natively_serializable<T> ||
                       to_string_serializable<T> ||
                       model_serializable<T> ||
                       model_container_serializable<T>;

using serialized_t = std::variant<long long,
                                  long double,
                                  bool,
                                  std::nullptr_t,
                                  std::string,
                                  std::shared_ptr<base_model>,
                                  std::vector<std::shared_ptr<base_model>>>;

struct serialized {
    enum {
        integer,
        floating_point,
        boolean,
        null,
        string,
        model,
        models
    } type;
    
    serialized_t value;
};
