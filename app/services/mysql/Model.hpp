#pragma once

#include "../database/Model.hpp"

namespace mysql {
    class model : public db::model,
                  public base_serializer {
    public:
        void save();
        void remove();
    };
}
