#include "Model.hpp"
#include "Connection.hpp"
#include "../tools/Format.hpp"

#include <stdexcept>

namespace mysql {
    // todo: protect against sql injections: use bind!
    // todo: take advantage of statement preparation!
    // todo: figure out why IDs are so random
    // todo: ID retrieval is not thread safe
    // todo: implement a way of inserting multiple rows simultaneously
    void model::save() {
        auto& session = connection::get_instance().session;
        auto& db      = connection::get_instance().db;
        auto  t       = db.getTable(table_name());

        if (!t.existsInDatabase()) {
            throw std::runtime_error(::format("Cannot save model: table \"{}\" does not exist in the database.", table_name()));
        }

        if (created) {
            auto update = t.update();

            for(auto& property : sorted_properties) {
                auto value = property.second->serialize_value();
                
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

            for(auto& property : sorted_properties) {
                if (index == 0) {
                    row.set(index++, 0);
                    continue;
                }

                auto value = property.second->serialize_value();

                switch(value.type) {
                    case serialized::integer:        row.set(index++,         std::get<long long     >(value.value)); break;
                    case serialized::floating_point: row.set(index++, (double)std::get<long double   >(value.value)); break;
                    case serialized::boolean:        row.set(index++,         std::get<bool          >(value.value)); break;
                    case serialized::null:           row.set(index++,         std::get<std::nullptr_t>(value.value)); break;
                    case serialized::string:         row.set(index++,         std::get<std::string   >(value.value)); break;
                }
            }

            t.insert().values(row).execute();

            id = session.sql("SELECT LAST_INSERT_ID();").execute().fetchOne()[0];
            
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
