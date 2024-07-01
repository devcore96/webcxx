#include "Table.hpp"

namespace db {
    std::vector<std::shared_ptr<model>> where_query::get() {
        return t.get(wheres);
    }
}
