#pragma once

#include <mysql-cppconn-8/mysqlx/xdevapi.h>

#include "../database/Transaction.hpp"

namespace mysql {
    class connection;

    class transaction : db::transaction {
    private:
        friend class connection;

        transaction(connection& conn);

        mysqlx::Session& session;

    public:
        void commit();
        void rollback();
    };
}

#include "Connection.hpp"
