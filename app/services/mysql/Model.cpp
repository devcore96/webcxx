#include "Model.hpp"
#include "Connection.hpp"
#include "../tools/Format.hpp"

#include <stdexcept>

namespace mysql {
    // todo: protect against sql injections: use bind!
    void model::save() {
        auto& session = connection::get_instance().session;
        auto& db      = connection::get_instance().db;
        auto  t       = db.getTable(table_name());

        if (!t.existsInDatabase()) {
            throw std::runtime_error(::format("Cannot save model: table \"{}\" does not exist in the database.", table_name()));
        }

        if (created) {
            auto update = t.update();

            for(auto& property : properties) {
                auto value = get_value(property.second);
                
                switch(value.type) {
                    case serialized::integer:        update.set(property.first, std::get<long long     >(value.value)); break;
                    case serialized::floating_point: update.set(property.first, std::get<long double   >(value.value)); break;
                    case serialized::boolean:        update.set(property.first, std::get<bool          >(value.value)); break;
                    case serialized::null:           update.set(property.first, std::get<std::nullptr_t>(value.value)); break;
                    case serialized::string:         update.set(property.first, std::get<std::string   >(value.value)); break;
                }
            }

            update.where(::format("id = {}", (size_t)properties["id"])).execute();
        } else {
            mysqlx::abi2::Row row;
            
            size_t index = 0;

            for(auto& property : properties) {
                auto value = get_value(property.second);
                
                switch(value.type) {
                    case serialized::integer:        row.set(index++, std::get<long long     >(value.value)); break;
                    case serialized::floating_point: row.set(index++, std::get<long double   >(value.value)); break;
                    case serialized::boolean:        row.set(index++, std::get<bool          >(value.value)); break;
                    case serialized::null:           row.set(index++, std::get<std::nullptr_t>(value.value)); break;
                    case serialized::string:         row.set(index++, std::get<std::string   >(value.value)); break;
                }
            }

            t.insert().values(row).execute();

            created = true;
        }
    }

    void model::remove() {
        if(!created)
            return;

        auto& session = connection::get_instance().session;
        auto& db      = connection::get_instance().db;
        auto  t       = db.getTable(table_name());

        if (!t.existsInDatabase()) {
            throw std::runtime_error(::format("Cannot remove model: table \"{}\" does not exist in the database.", table_name()));
        }

        t.remove().where(::format("id = {}", (size_t)properties["id"])).execute();
    }
}
