#pragma once

#include "../serialization/Model.hpp"

#include <string>

namespace db {
    class table;

    class model : public base_model {
    protected:
        friend class table;

        bool created = false;

    public:
        model() : id(this, "id", 0) { }

        property<size_t> id;

        virtual void save() = 0;
        virtual void remove() = 0;

        virtual std::string table_name() const = 0;

        virtual table* get_table() = 0;
    };

    /* class joined_model : public base_model {
    protected:
        property<std::vector<std::shared_ptr<base_model>>> models;

    public:
        joined_model() : models(this, "models") { }

        std::vector<std::shared_ptr<base_model>>& get_models() {
            return models;
        }
    }; */
}

#include "Table.hpp"
