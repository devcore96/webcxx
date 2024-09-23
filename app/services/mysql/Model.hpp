#pragma once

#include "../database/Model.hpp"
#include <vector>
#include <memory>

namespace mysql {
    class model : public db::model {
    public:
        void save();
        void remove();
        
        // Declare the save_multiple method to match the definition in Model.cpp
        void save_multiple(const std::vector<std::shared_ptr<model>>& models);

        db::table* get_table() { return nullptr; }
    };
}
