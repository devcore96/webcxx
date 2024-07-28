#pragma once

#include <mysql-cppconn-8/mysqlx/xdevapi.h>
#include <mutex>

#include "../database/Transaction.hpp"

namespace mysql {
    class connection;

    class transaction : db::transaction {
    private:
        friend class connection;

        transaction(connection& conn);

        mysqlx::Session& session;
        bool unlocked = false;

        static std::mutex mutex;

    public:
        ~transaction();


        void commit();
        void rollback();
    };
}

#include "Connection.hpp"
