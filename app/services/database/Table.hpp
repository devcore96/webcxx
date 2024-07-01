#pragma once

#include "Model.hpp"

#include <memory>
#include <vector>
#include <string>
#include <sstream>

namespace db {
    class base_table {
    protected:
        std::string name;

    public:
        virtual void create() = 0;
    };

    struct where_query_t {
        std::string key;
        std::string value;
        std::string query_operator;
    };

    class table;

    class where_query {
    private:
        friend class table;

        table& t;
        std::vector<where_query_t> wheres;

    public:
        where_query(table& t) : t(t) { }
        template<typename T>
        where_query& where(std::string key, T value, std::string query_operator) {
            std::ostringstream stream;
            stream << value;

            wheres.push_back(where_query_t {
                .key = key,
                .value = stream.str(),
                .query_operator = query_operator
            });

            return *this;
        }

        std::vector<std::shared_ptr<model>> get();
    };

    class table : public base_table {
    protected:
        friend class where_query;

        std::string name;

        virtual std::vector<std::shared_ptr<model>> get(std::vector<where_query_t> wheres) const = 0;

    public:
        template<std::derived_from<model> Model>
        table() : name((Model { }).table_name()) { }

        virtual std::vector<std::shared_ptr<model>> all() const = 0;
        virtual void create() const = 0;
        virtual void destroy() const = 0;

        template<typename T>
        where_query where(std::string key, T value, std::string query_operator) const {
            where_query where { *this };

            std::ostringstream stream;
            stream << value;

            where.wheres.push_back(where_query_t {
                .key = key,
                .value = stream.str(),
                .query_operator = query_operator
            });

            return where;
        }
    };
}
