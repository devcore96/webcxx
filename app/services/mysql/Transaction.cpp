#include "Transaction.hpp"

namespace mysql {
    transaction::transaction(connection& conn) : session(conn.session) {
        session.startTransaction();
    }

    void transaction::commit() {
        session.commit();
    }

    void transaction::rollback() {
        session.rollback();
    }
}
