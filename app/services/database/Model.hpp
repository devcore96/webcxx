#pragma once

#include "../serialization/Model.hpp"

namespace db {
    class model : public base_model {
    protected:
        std::string table_name;
        bool created = false;

    public:
        virtual void save() = 0;
    };
}
