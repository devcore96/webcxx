#pragma once

#include "../database/Table.hpp"
#include "../tools/Container.hpp"

#include "Model.hpp"
#include "Connection.hpp"
#include <iostream>

namespace mysql {
    class model;
    class joined_table;
    
    template<class Model>
    class table : public db::table,
                         base_serializer {
    protected:
        size_t next_id = 0;

        friend class model;

        std::vector<std::shared_ptr<db::model>> get(std::vector<db::where_query_t>    wheres,
                                                    std::vector<db::order_by_query_t> order_bys,
                                                    size_t                            limit) const {
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

            for (auto& condition : order_bys) {
                select = select.orderBy(std::format("{} {}", condition.key, (condition.asc ? "ASC" : "DESC")));
            }

            auto result = select.limit(limit).execute();
            auto& columns = result.getColumns();

            for(auto row : result) {
                std::shared_ptr<db::model> m = std::make_shared<Model>();

                // todo: change this. I hate this.
                auto& properties = get_properties(*m);

                size_t i = 0;

                for (auto& column : columns) {
                    auto& value = row.get(i++);

                    switch (value.getType()) {
                        case mysqlx::abi2::r0::Value::Type::VNULL:  properties[column.getColumnName()]->deserialize_value(serialized { .type = serialized::null,           .value =                       nullptr }); break;
                        case mysqlx::abi2::r0::Value::Type::UINT64: properties[column.getColumnName()]->deserialize_value(serialized { .type = serialized::integer,        .value = (ptrdiff_t)(     size_t)value }); break;
                        case mysqlx::abi2::r0::Value::Type::INT64:  properties[column.getColumnName()]->deserialize_value(serialized { .type = serialized::integer,        .value =            (  ptrdiff_t)value }); break;
                        case mysqlx::abi2::r0::Value::Type::FLOAT:  properties[column.getColumnName()]->deserialize_value(serialized { .type = serialized::floating_point, .value =            (      float)value }); break;
                        case mysqlx::abi2::r0::Value::Type::DOUBLE: properties[column.getColumnName()]->deserialize_value(serialized { .type = serialized::floating_point, .value =            (     double)value }); break;
                        case mysqlx::abi2::r0::Value::Type::BOOL:   properties[column.getColumnName()]->deserialize_value(serialized { .type = serialized::boolean,        .value =            (       bool)value }); break;
                        case mysqlx::abi2::r0::Value::Type::STRING: properties[column.getColumnName()]->deserialize_value(serialized { .type = serialized::string,         .value =            (std::string)value }); break;
                    }
                }

                models.push_back(m);
            }

            return models;
        }

        void remove(std::vector<db::where_query_t>    wheres,
                    std::vector<db::order_by_query_t> order_bys,
                    size_t                            limit) const {
            auto& session = connection::get_instance().session;
            auto& db      = connection::get_instance().db;
            auto  table   = db.getTable(name);

            if (!table.existsInDatabase()) {
                throw std::runtime_error(std::format("Cannot remove models: table \"{}\" does not exist in the database.", name));
            }

            std::vector<std::shared_ptr<db::model>> models;

            auto remove = table.remove();

            for (auto& condition : wheres) {
                remove = remove.where(std::format("{} {} {}", condition.key, condition.query_operator, condition.value));
            }

            remove.limit(limit).execute();
        }

        std::shared_ptr<db::joined_table> join(const base_table& that,
                                               std::string this_key,
                                               std::string that_key,
                                               db::join_mode_t mode) const {
            return std::make_shared<joined_table>(this, &that, this_key, that_key, mode);
        }

    public:
        table() : db::table(Model { }) { };
        table(const table& ) = default;
        table(      table&&) = default;

        table& operator=(const table& ) = default;
        table& operator=(      table&&) = default;
        
        void insert(std::vector<std::shared_ptr<db::model>> models) {
            auto& session = connection::get_instance().session;
            auto& db      = connection::get_instance().db;
            auto  table   = db.getTable(name);

            if (!table.existsInDatabase()) {
                throw std::runtime_error(std::format("Cannot insert models: table \"{}\" does not exist in the database.", name));
            }

            std::vector<mysqlx::abi2::Row> rows;

            auto insert_statement = table.insert();

            for (auto& model : models) {
                mysqlx::abi2::Row row;
                
                size_t index = 0;

                for(auto& property : get_sorted_properties(*model)) {
                    auto value = property.second->serialize_value();

                    switch(value.type) {
                        case serialized::integer:        row.set(index++,         std::get<long long     >(value.value)); break;
                        case serialized::floating_point: row.set(index++, (double)std::get<long double   >(value.value)); break;
                        case serialized::boolean:        row.set(index++,         std::get<bool          >(value.value)); break;
                        case serialized::null:           row.set(index++,         std::get<std::nullptr_t>(value.value)); break;
                        case serialized::string:         row.set(index++,         std::get<std::string   >(value.value)); break;
                    }
                }

                model->id = get_next_id();

                insert_statement = insert_statement.values(row);
            }

            insert_statement.execute();

            size_t last = session.sql("SELECT LAST_INSERT_ID();").execute().fetchOne()[0];

            size_t i = 1;

            for (auto& model : models) {
                if (model->id == 0) {
                    model->id = last - models.size() + (i++);
                } else i++;
            }
        }

        void remove(std::vector<std::shared_ptr<db::model>> models) const {
            auto& session = connection::get_instance().session;
            auto& db      = connection::get_instance().db;
            auto  table   = db.getTable(name);

            if (!table.existsInDatabase()) {
                throw std::runtime_error(std::format("Cannot remove models: table \"{}\" does not exist in the database.", name));
            }

            std::string condition = "";
            size_t i = 0;

            for (auto& model : models) {
                if (!condition.empty()) {
                    condition += ",";
                }

                condition += std::to_string((size_t)model->id);
            }

            table.remove().where("id IN [" + condition + "]").execute();
        }

        void create() const {
            // todo
        }

        void destroy() const {
            // todo
        }

        void clear() const {
            auto& session = connection::get_instance().session;
            auto& db      = connection::get_instance().db;
            auto  table   = db.getTable(name);

            if (!table.existsInDatabase()) {
                throw std::runtime_error(std::format("Cannot remove models: table \"{}\" does not exist in the database.", name));
            }

            table.remove().where("id > 0").execute();
        }

        size_t get_next_id(bool force_update = false) {
            if (next_id == 0 || force_update)
            {
                auto& session = connection::get_instance().session;
                auto& db      = connection::get_instance().db;
                auto  table   = db.getTable(name);

                try {
                    next_id = table.select("MAX(id)").execute().fetchOne().get(0);
                } catch(mysqlx::abi2::r0::Error& e) {
                    next_id = 0;
                }
            }

            return ++next_id;
        }
    };

    class joined_table : public db::joined_table {
    protected:
        std::vector<std::shared_ptr<db::model>> get(std::vector<db::where_query_t>    wheres,
                                                    std::vector<db::order_by_query_t> order_bys,
                                                    size_t                            limit) const {
            /* auto& session = connection::get_instance().session;
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

            for (auto& condition : order_bys) {
                select = select.orderBy(std::format("{} {}", condition.key, (condition.asc ? "ASC" : "DESC")));
            }

            auto result = select.limit(limit).execute();
            auto& columns = result.getColumns();

            for(auto row : result) {
                std::shared_ptr<db::model> m = std::make_shared<Model>();

                // todo: change this. I hate this.
                auto& properties = get_properties(*m);

                size_t i = 0;

                for (auto& column : columns) {
                    auto& value = row.get(i++);

                    switch (value.getType()) {
                        case mysqlx::abi2::r0::Value::Type::VNULL:  properties[column.getColumnName()]->deserialize_value(serialized { .type = serialized::null,           .value =                       nullptr }); break;
                        case mysqlx::abi2::r0::Value::Type::UINT64: properties[column.getColumnName()]->deserialize_value(serialized { .type = serialized::integer,        .value = (ptrdiff_t)(     size_t)value }); break;
                        case mysqlx::abi2::r0::Value::Type::INT64:  properties[column.getColumnName()]->deserialize_value(serialized { .type = serialized::integer,        .value =            (  ptrdiff_t)value }); break;
                        case mysqlx::abi2::r0::Value::Type::FLOAT:  properties[column.getColumnName()]->deserialize_value(serialized { .type = serialized::floating_point, .value =            (      float)value }); break;
                        case mysqlx::abi2::r0::Value::Type::DOUBLE: properties[column.getColumnName()]->deserialize_value(serialized { .type = serialized::floating_point, .value =            (     double)value }); break;
                        case mysqlx::abi2::r0::Value::Type::BOOL:   properties[column.getColumnName()]->deserialize_value(serialized { .type = serialized::boolean,        .value =            (       bool)value }); break;
                        case mysqlx::abi2::r0::Value::Type::STRING: properties[column.getColumnName()]->deserialize_value(serialized { .type = serialized::string,         .value =            (std::string)value }); break;
                    }
                }

                models.push_back(m);
            }

            return models; */

            return { };
        }

        void remove(std::vector<db::where_query_t>    wheres,
                    std::vector<db::order_by_query_t> order_bys,
                    size_t                            limit) const {
            // todo
        }

        std::shared_ptr<db::joined_table> join(const base_table& that,
                                               std::string this_key,
                                               std::string that_key,
                                               db::join_mode_t mode) const {
            return std::make_shared<joined_table>(this, &that, this_key, that_key, mode);
        }

        bool joined = false;

    public:
        joined_table(const base_table* this_table,
                     const base_table* that_table,
                           std::string this_key,
                           std::string that_key,
                       db::join_mode_t mode) : db::joined_table(this_table,
                                                                that_table,
                                                                this_key,
                                                                that_key,
                                                                mode) { }
    };
}
