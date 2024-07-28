#include "Transaction.hpp"

namespace mysql {
    std::mutex transaction::mutex;

    transaction::transaction(connection& conn) : session(conn.session) {
        mutex.lock();
        session.startTransaction();
    }

    transaction::~transaction() {
        if (!unlocked) {
            mutex.unlock();
        }
    }

    void transaction::commit() {
        session.commit();

        unlocked = true;
        mutex.unlock();
    }

    void transaction::rollback() {
        session.rollback();

        unlocked = true;
        mutex.unlock();
    }
}
