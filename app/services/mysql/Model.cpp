#include "Model.hpp"
#include "Connection.hpp"
#include "../tools/Format.hpp"

#include <stdexcept>
#include <variant>
#include <vector>
#include <memory>

namespace mysql {

    // Assuming serialized_t is defined as:
    // using serialized_t = std::variant<long long, long double, bool, std::nullptr_t, std::string, std::shared_ptr<base_model>, std::vector<std::shared_ptr<base_model>>>;

    // Utility function to extract ID from properties
    long long extract_id(const std::map<std::string, base_property*>& properties) {
        auto it = properties.find("id");
        if (it == properties.end() || it->second == nullptr) {
            throw std::runtime_error("ID property is missing.");
        }

        auto serialized_id = it->second->serialize_value();
        long long id_value = 0;
        bool id_extracted = false;

        std::visit([&id_value, &id_extracted](auto&& val) {
            using T = std::decay_t<decltype(val)>;
            if constexpr (std::is_same_v<T, long long>) {
                id_value = val;
                id_extracted = true;
            }
            // Add other types if ID can have different types
        }, serialized_id.value);

        if (!id_extracted) {
            throw std::runtime_error("ID property has an unexpected type.");
        }

        return id_value;
    }

    void model::save() {
        auto& session = connection::get_instance().session;
        auto& db      = connection::get_instance().db;
        auto  t       = db.getTable(table_name());

        if (!t.existsInDatabase()) {
            throw std::runtime_error(::format("Cannot save model: table \"{}\" does not exist in the database.", table_name()));
        }

        if (created) {
            // Prepare the update statement with parameter binding
            auto update = t.update();

            for(auto& property : sorted_properties) {
                auto value = property.second->serialize_value();
                
                // Use std::visit to handle the variant and set parameters
                std::visit([&update, &property](auto&& val) {
                    using T = std::decay_t<decltype(val)>;
                    if constexpr (std::is_same_v<T, long long>) {
                        update.set(property.first, val);
                    }
                    else if constexpr (std::is_same_v<T, long double>) {
                        update.set(property.first, static_cast<double>(val)); // MySQL expects double
                    }
                    else if constexpr (std::is_same_v<T, bool>) {
                        update.set(property.first, val);
                    }
                    else if constexpr (std::is_same_v<T, std::nullptr_t>) {
                        update.set(property.first, nullptr);
                    }
                    else if constexpr (std::is_same_v<T, std::string>) {
                        update.set(property.first, val);
                    }
                    // Handle other types if necessary
                }, value.value);
            }

            // Extract ID safely
            long long id_value = extract_id(properties);

            // Use bound parameter to prevent SQL injection
            update.where("id = :id").bind("id", id_value).execute();
        } else {
            mysqlx::Row row;

            size_t index = 0;

            for(auto& property : sorted_properties) {
                auto value = property.second->serialize_value();

                // Set the row value based on the variant's type using std::visit
                std::visit([&row, &index](auto&& val) {
                    using T = std::decay_t<decltype(val)>;
                    if constexpr (std::is_same_v<T, long long>) {
                        row.set(index++, val);
                    }
                    else if constexpr (std::is_same_v<T, long double>) {
                        row.set(index++, static_cast<double>(val)); // MySQL expects double
                    }
                    else if constexpr (std::is_same_v<T, bool>) {
                        row.set(index++, val);
                    }
                    else if constexpr (std::is_same_v<T, std::nullptr_t>) {
                        row.set(index++, nullptr);
                    }
                    else if constexpr (std::is_same_v<T, std::string>) {
                        row.set(index++, val);
                    }
                    // Handle other types if necessary
                }, value.value);
            }

            // Insert the row using prepared statements
            auto res = t.insert().values(row).execute();

            // Retrieve the auto-incremented ID in a thread-safe manner
            id = res.getAutoIncrementValue(); // Ensure 'id' is of a compatible type

            created = true;
        }
    }

    void model::remove() {
        if(!created)
            return;

        auto& db = connection::get_instance().db;
        auto t   = db.getTable(table_name());

        if (!t.existsInDatabase()) {
            throw std::runtime_error(::format("Cannot remove model: table \"{}\" does not exist in the database.", table_name()));
        }

        // Extract ID safely
        long long id_value = extract_id(properties);

        // Use bound parameter to prevent SQL injection
        t.remove().where("id = :id").bind("id", id_value).execute();
    }

    // Implementing batch insertion
    void model::save_multiple(const std::vector<std::shared_ptr<model>>& models) {
        if (models.empty()) return;

        auto& db = connection::get_instance().db;
        auto t = db.getTable(table_name());

        if (!t.existsInDatabase()) {
            throw std::runtime_error(::format("Cannot save models: table \"{}\" does not exist in the database.", table_name()));
        }

        // Insert operation starts here, t.insert() returns a TableInsert statement
        auto insert_stmt = t.insert();

        for(auto& mdl_ptr : models) {
            mysqlx::Row row;
            size_t index = 0;

            for(auto& property : mdl_ptr->sorted_properties) {
                auto value = property.second->serialize_value();

                // Set the row value based on the variant's type using std::visit
                std::visit([&row, &index](auto&& val) {
                    using T = std::decay_t<decltype(val)>;
                    if constexpr (std::is_same_v<T, long long>) {
                        row.set(index++, val);
                    }
                    else if constexpr (std::is_same_v<T, long double>) {
                        row.set(index++, static_cast<double>(val)); // MySQL expects double
                    }
                    else if constexpr (std::is_same_v<T, bool>) {
                        row.set(index++, val);
                    }
                    else if constexpr (std::is_same_v<T, std::nullptr_t>) {
                        row.set(index++, nullptr);
                    }
                    else if constexpr (std::is_same_v<T, std::string>) {
                        row.set(index++, val);
                    }
                }, value.value);
            }

            // Add the row to the batch insert
            insert_stmt.values(row);
        }

        // Execute the batch insert
        auto res = insert_stmt.execute();

        // Assign the auto-incremented IDs to the models
        long long first_id = res.getAutoIncrementValue();
        for(size_t i = 0; i < models.size(); ++i) {
            models[i]->id = first_id + i;
            models[i]->created = true;
        }
    }

}
