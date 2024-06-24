#pragma once

#include "Model.hpp"

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

    template<std::derived_from<model> Model>
    class table;

    template<std::derived_from<model> Model>
    class where_query {
    private:
        std::vector<where_query_t> wheres;

        friend class table<Model>;

    public:
        template<typename T>
        where_query<Model>& where(std::string key, T value, std::string query_operator) {
            std::ostringstream stream;
            stream << value;

            wheres.push_back(where_query_t {
                .key = key,
                .value = stream.str(),
                .query_operator = query_operator
            });

            return *this;
        }

        virtual std::vector<Model> get() = 0;
    };

    template<std::derived_from<model> Model>
    class table : public base_table {
    public:
        virtual std::vector<Model> all() = 0;

        template<typename T>
        where_query<Model> where(std::string key, T value, std::string operator) {
            where_query<Model> where;

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
