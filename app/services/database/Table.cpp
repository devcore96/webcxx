#include "Table.hpp"

namespace db {
    std::vector<std::shared_ptr<model>> IExecutable::get() const {
        return t.get(wheres,
                     order_bys,
                     limit);
    }

    std::vector<std::shared_ptr<model>> base_table::all() const {
        return get({ },
                   { },
                   -1);
    }
}
