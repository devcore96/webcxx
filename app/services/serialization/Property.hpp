#pragma once

#include "Serializable.hpp"

#include <stdexcept>

class base_serializer;
class base_model;

class base_property {
protected:
    friend class base_serializer;

    std::string key;
    base_model* model;

    void register_property();

    base_property(std::string key, base_model* model) : key(key), model(model) { }

    base_property(const base_property & ) = default;
    base_property(      base_property &&) = default;

    base_property& operator=(const base_property&  value) = default;
    base_property& operator=(      base_property&& value) = default;

public:
    virtual serialized serialize_value() = 0;
    virtual void deserialize_value(serialized val) = 0;
};

template<serializable T>
class property : public base_property {
private:
    T value;

public:
    serialized serialize_value_b() noexcept(true) requires(std::same_as<bool,           T>) { return serialized { serialized::boolean,                      value  }; }
    serialized serialize_value_f() noexcept(true) requires(std::floating_point         <T>) { return serialized { serialized::floating_point, (long double) value  }; }
    serialized serialize_value_i() noexcept(true) requires(std::integral               <T>) { return serialized { serialized::integer,        (long   long) value  }; }
    serialized serialize_value_n() noexcept(true) requires(std::same_as<std::nullptr_t, T>) { return serialized { serialized::null,                         value  }; }
    serialized serialize_value_s() noexcept(true) requires(to_string_serializable      <T>) { return serialized { serialized::string,           std::string(value) }; }

    serialized serialize_value() {
        if constexpr(std::same_as<bool,           T>) return serialize_value_b();
        if constexpr(std::floating_point         <T>) return serialize_value_f();
        if constexpr(std::integral               <T>) return serialize_value_i();
        if constexpr(std::same_as<std::nullptr_t, T>) return serialize_value_n();
        if constexpr(to_string_serializable      <T>) return serialize_value_s();

        return { };
    }

    void deserialize_value_b(serialized val) requires(std::same_as<bool,           T>) { if (val.type != serialized::boolean       ) throw std::bad_cast(); value = std::get<bool          >(val.value); }
    void deserialize_value_f(serialized val) requires(std::floating_point         <T>) { if (val.type != serialized::floating_point) throw std::bad_cast(); value = std::get<long double   >(val.value); }
    void deserialize_value_i(serialized val) requires(std::integral               <T>) { if (val.type != serialized::integer       ) throw std::bad_cast(); value = std::get<long long     >(val.value); }
    void deserialize_value_n(serialized val) requires(std::same_as<std::nullptr_t, T>) { if (val.type != serialized::null          ) throw std::bad_cast(); value = std::get<std::nullptr_t>(val.value); }
    void deserialize_value_s(serialized val) requires(to_string_serializable      <T>) { if (val.type != serialized::string        ) throw std::bad_cast(); value = std::get<std::string   >(val.value); }

    void deserialize_value(serialized val) {
        if constexpr(std::same_as<bool,           T>) { deserialize_value_b(val); return; }
        if constexpr(std::floating_point         <T>) { deserialize_value_f(val); return; }
        if constexpr(std::integral               <T>) { deserialize_value_i(val); return; }
        if constexpr(std::same_as<std::nullptr_t, T>) { deserialize_value_n(val); return; }
        if constexpr(to_string_serializable      <T>) { deserialize_value_s(val); return; }
    }

    property(const property<T>&  value) requires (std::copy_constructible<T>) = default;
    property(      property<T>&& value) requires (std::move_constructible<T>) = default;

    property(base_model* model, std::string key)                  requires (std::default_initializable<T>) : base_property(key, model)               { register_property(); }
    property(base_model* model, std::string key, const T&  value) requires (std::copy_constructible   <T>) : base_property(key, model), value(value) { register_property(); }
    property(base_model* model, std::string key,       T&& value) requires (std::move_constructible   <T>) : base_property(key, model), value(value) { register_property(); }

    property& operator=(const property<T>&  value) requires (std::copy_constructible<T>) = default;
    property& operator=(      property<T>&& value) requires (std::move_constructible<T>) = default;
    property& operator=(const          T &  val  ) requires (std::copy_constructible<T>) { value = val; return *this; }
    property& operator=(               T && val  ) requires (std::move_constructible<T>) { value = val; return *this; }

    operator T&() { return value; }

    template<to_string_serializable S>
    bool operator==(S str) requires (to_string_serializable<T>) { return std::string(value) == str; }
};

#include "Model.hpp"
