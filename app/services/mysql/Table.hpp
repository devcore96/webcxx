#pragma once

#include "../database/Table.hpp"
#include "../tools/Container.hpp"

#include "Model.hpp"
#include "Connection.hpp"

namespace mysql {
    class model;
    
    template<class Model>
    class table : public db::table,
                         base_serializer {
    protected:
        friend class model;

        std::vector<std::shared_ptr<db::model>> get(std::vector<db::where_query_t> wheres) const {
            auto& session = connection::get_instance().session;
            auto& db      = connection::get_instance().db;
            auto  table   = db.getTable(name);

            if (!table.existsInDatabase()) {
                throw std::runtime_error(std::format("Cannot select models: table \"{}\" does not exist in the database.", name));
            }

            std::vector<std::shared_ptr<db::model>> models;

            auto select = table.select();

            for (auto& condition : wheres) {
                select = select.where(std::format("{} {} {}", condition.key, condition.query_operator, condition.value));
            }

            auto result = select.execute();
            auto& columns = result.getColumns();

            for(auto row : result) {
                std::shared_ptr<db::model> m = std::make_shared<Model>();

                // todo: change this. I hate this.
                auto& properties = get_properties(*m);

                size_t i = 0;

                for (auto& column : columns) {
                    auto& value = row.get(i);

                    switch (value.getType()) {
                        case mysqlx::abi2::r0::Value::Type::VNULL:  set_value(properties[column.getColumnName()], serialized { .type = serialized::null,           .value =                       nullptr }); break;
                        case mysqlx::abi2::r0::Value::Type::UINT64: set_value(properties[column.getColumnName()], serialized { .type = serialized::integer,        .value = (ptrdiff_t)(     size_t)value }); break;
                        case mysqlx::abi2::r0::Value::Type::INT64:  set_value(properties[column.getColumnName()], serialized { .type = serialized::integer,        .value =            (  ptrdiff_t)value }); break;
                        case mysqlx::abi2::r0::Value::Type::FLOAT:  set_value(properties[column.getColumnName()], serialized { .type = serialized::floating_point, .value =            (      float)value }); break;
                        case mysqlx::abi2::r0::Value::Type::DOUBLE: set_value(properties[column.getColumnName()], serialized { .type = serialized::floating_point, .value =            (     double)value }); break;
                        case mysqlx::abi2::r0::Value::Type::BOOL:   set_value(properties[column.getColumnName()], serialized { .type = serialized::boolean,        .value =            (       bool)value }); break;
                        case mysqlx::abi2::r0::Value::Type::STRING: set_value(properties[column.getColumnName()], serialized { .type = serialized::string,         .value =            (std::string)value }); break;
                    }
                }

                models.push_back(m);
            }

            return models;
        }

    public:
        table() : db::table(Model { }) { };
        table(const table& ) = default;
        table(      table&&) = default;

        table& operator=(const table& ) = default;
        table& operator=(      table&&) = default;

        std::vector<std::shared_ptr<db::model>> all() const {
            return get({});
        }

        void create() const {
            // todo
        }

        void destroy() const {
            // todo
        }
    };
}
