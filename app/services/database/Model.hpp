#pragma once

#include "../serialization/Model.hpp"

#include <string>

namespace db {
    class table;

    class model : public base_model {
    protected:
        virtual std::string table_name() const = 0;

        friend class table;

        bool created = false;

    public:
        model() : id(this, "id") { }

        property<size_t> id;

        virtual void save() = 0;
        virtual void remove() = 0;
    };
}

#include "Table.hpp"
